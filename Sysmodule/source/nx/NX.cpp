#include "Log.hpp"
#include "nx/Audio.hpp"
#include "nx/File.hpp"
#include "nx/NX.hpp"
#include <mutex>
#include <switch.h>
#include <thread>
#include <unordered_map>

namespace NX {
    // Helper to log messages
    void logError(const std::string & service, const Result & rc) {
        Log::writeError("[NX] Failed to initialize " + service + ": " + std::to_string(rc));
    }

    // Variables indicating if each service was initialized
    static bool audrenInitialized = false;
    static bool fsInitialized = false;
    static bool hidInitialized = false;
    static bool gpioInitialized = false;
    static bool pscmInitialized = false;
    static bool smInitialized = false;

    // Starts all needed services
    bool startServices() {
        // Prevent starting twice
        if (audrenInitialized || fsInitialized || gpioInitialized || hidInitialized || pscmInitialized || smInitialized) {
            return true;
        }
        Result rc;

        // SM
        rc = smInitialize();
        if (R_SUCCEEDED(rc)) {
            smInitialized = true;

        } else {
            return false;
        }

        // FS
        rc = fsInitialize();
        if (R_SUCCEEDED(rc)) {
            fsdevMountSdmc();
            fsInitialized = File::initializeService();
        }
        if (!fsInitialized) {
            return false;
        }

        // Open log file
        Log::openFile("/switch/TriPlayer/sysmodule.log", Log::Level::Warning);

        // Set system version
        rc = setsysInitialize();
        if (R_SUCCEEDED(rc)) {
            SetSysFirmwareVersion fw;
            setsysGetFirmwareVersion(&fw);
            hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
            setsysExit();

        } else {
            logError("system version", rc);
        }

        // Audio
        constexpr AudioRendererConfig audrenCfg = {
            .output_rate     = AudioRendererOutputRate_48kHz,
            .num_voices      = 4,
            .num_effects     = 0,
            .num_sinks       = 1,
            .num_mix_objs    = 1,
            .num_mix_buffers = 2,
        };
        rc = audrenInitialize(&audrenCfg);
        if (R_SUCCEEDED(rc)) {
            Audio * audio = Audio::getInstance();
            audrenStartAudioRenderer();
            audrenInitialized = audio->initialized();
        }
        if (!audrenInitialized) {
            logError("audio", rc);
            return false;
        }

        // GPIO
        rc = gpioInitialize();
        if (R_SUCCEEDED(rc)) {
            gpioInitialized = true;

        } else {
            logError("gpio", rc);
        }

        // HID
        rc = hidInitialize();
        if (R_SUCCEEDED(rc)) {
            hidInitialized = true;

        } else {
            logError("hid", rc);
        }

        // PSC
        rc = pscmInitialize();
        if (R_SUCCEEDED(rc)) {
            pscmInitialized = true;

        } else {
            logError("pscm", rc);
        }

        // We don't care if gpio, hid or pscm don't initialize
        return true;
    }

    // Stops all started services (in reverse order)
    void stopServices() {
        // PSC
        if (pscmInitialized) {
            pscmExit();
            pscmInitialized = false;
        }

        if (hidInitialized) {
            hidExit();
            hidInitialized = false;
        }

        // GPIO
        if (gpioInitialized) {
            gpioExit();
            gpioInitialized = false;
        }

        // Audio
        if (audrenInitialized) {
            audrenStopAudioRenderer();
            delete Audio::getInstance();
            audrenExit();
            audrenInitialized = false;
        }

        // Close log
        Log::closeFile();

        // FS
        if (fsInitialized) {
            File::closeService();
            fsdevUnmountAll();
            fsExit();
            fsInitialized = false;
        }

        // SM
        if (smInitialized) {
            smExit();
            smInitialized = false;
        }
    }

    namespace Fs {
        void setHighPriority(const bool b) {
            fsSetPriority(b ? FsPriority_Realtime : FsPriority_Normal);
        }
    };

    namespace Gpio {
        static bool gpioPrepared = false;       // Set true if ready to poll pad
        static GpioPadSession gpioSession;      // Current session used to read pad
        static GpioValue gpioLastValue;         // Last read value (low if plugged in, high if not)

        // Open session for headset pad
        bool prepare() {
            // Don't prepare if service not initialized
            if (!gpioInitialized) {
                logError("gpio pad", -1);
                return false;
            }

            // Prevent preparing twice
            if (gpioPrepared) {
                return true;
            }

            // Open for GPIO pad 0x15, which indicates if headset is connected
            gpioLastValue = GpioValue_High;
            Result rc = gpioOpenSession(&gpioSession, (GpioPadName)0x15);
            if (R_SUCCEEDED(rc)) {
                gpioPrepared = true;
                return true;
            }

            logError("gpio pad", -2);
            return false;
        }

        // Close opened session
        void cleanup() {
            if (gpioPrepared) {
                gpioPadClose(&gpioSession);
                gpioPrepared = false;
            }
        }

        // Return if the headset was just unplugged
        bool headsetUnplugged() {
            // Don't attempt to check if not prepared
            if (!gpioPrepared) {
                return false;
            }

            // Get current value
            GpioValue currentValue;
            Result rc = gpioPadGetValue(&gpioSession, &currentValue);
            if (R_FAILED(rc)) {
                return false;
            }

            // If we have changed from Low to High, the headset was unplugged
            bool unplugged = false;
            if (gpioLastValue == GpioValue_Low && currentValue == GpioValue_High) {
                unplugged = true;
            }
            gpioLastValue = currentValue;
            return unplugged;
        }
    };

    namespace Hid {
        static PadState hidPad;                                     // Pad object
        static bool hidPrepared = false;                            // Set true if prepared

        bool prepare() {
            padConfigureInput(1, HidNpadStyleSet_NpadStandard);
            padInitializeDefault(&hidPad);
            hidPrepared = true;
            return true;
        }

        void cleanup() {
            // Don't need to clean up anything
        }

        bool comboPressed(const std::vector<Button> & buttons) {
            // Scan input first
            padUpdate(&hidPad);
            uint64_t pressed = padGetButtons(&hidPad);

            // Convert combo to same format
            uint64_t combo = 0;
            for (const Button & button : buttons) {
                combo |= (1U << static_cast<int>(button));
            }

            // Check if bits match (indicates combo is pressed)
            return ((pressed & combo) == combo);
        }
    };

    namespace Psc {
        constexpr u32 pscDependencies[] = {PscPmModuleId_Audio};    // Our dependencies
        constexpr PscPmModuleId pscModuleId = (PscPmModuleId)690;   // Our ID
        static PscPmModule pscModule;                               // Module to listen for events with
        static bool pscPrepared = false;                            // Set true if ready to handle event
        static std::function<void()> pscSleepFunc = nullptr;        // Callback when entering sleep
        static std::function<void()> pscWakeFunc = nullptr;         // Callback when waking up

        // Create module to listen with
        bool prepare() {
            // Don't prepare if service not initialized
            if (!pscmInitialized) {
                logError("psc module", -1);
                return false;
            }

            // Prevent preparing twice
            if (pscPrepared) {
                return true;
            }

            // Create module
            Result rc = pscmGetPmModule(&pscModule, pscModuleId, pscDependencies, sizeof(pscDependencies)/sizeof(u32), true);
            if (R_SUCCEEDED(rc)) {
                pscPrepared = true;
                return true;
            }

            logError("psc module", -2);
            return false;
        }

        // Delete created module
        void cleanup() {
            if (pscPrepared) {
                pscPmModuleFinalize(&pscModule);
                pscPmModuleClose(&pscModule);
                pscPrepared = false;
            }
        }

        void monitor(const size_t ms) {
            size_t ns = 1000000 * ms;

            // Don't wait for event if not prepared
            if (!pscPrepared) {
                svcSleepThread(ns);
                return;
            }

            // Wait for an event
            Result rc = eventWait(&pscModule.event, ns);
            if (R_VALUE(rc) == KERNELRESULT(TimedOut)) {
                return;

            } else if (R_VALUE(rc) == KERNELRESULT(Cancelled)) {
                svcSleepThread(ns);
                return;
            }

            // If there's an event, fetch the state
            PscPmState eventState;
            u32 flags;
            rc = pscPmModuleGetRequest(&pscModule, &eventState, &flags);
            if (R_FAILED(rc)) {
                return;
            }

            // Call any related callbacks
            switch (eventState) {
                case PscPmState_ReadySleep:
                    if (pscSleepFunc != nullptr) {
                        pscSleepFunc();
                    }
                    break;

                case PscPmState_ReadyAwaken:
                    if (pscWakeFunc != nullptr) {
                        pscWakeFunc();
                    }
                    break;

                // Do nothing for other types
                default:
                    break;
            }

            // Acknowledge that we've handled the event
            pscPmModuleAcknowledge(&pscModule, eventState);
        }

        void setSleepFunc(const std::function<void()> & f) {
            pscSleepFunc = f;
        }

        void setWakeFunc(const std::function<void()> & f) {
            pscWakeFunc = f;
        }
    };

    // I wanted to use libnx's API for threads but apparently that causes a Data Abort when a thread's
    // function returns (like literally after the last line)
    namespace Thread {
        static std::unordered_map<std::string, std::thread> threads;    // Map from name/id to thread object
        static std::mutex threadMutex;                                  // Mutex protecting map

        bool create(const std::string & id, void(*func)(void *), void * arg, const size_t size) {
            std::scoped_lock<std::mutex> mtx(threadMutex);

            // Don't start if thread exists
            if (threads.count(id) > 0) {
                return false;
            }

            // Create thread and emplace in map
            threads.emplace(id, std::thread(func, arg));
            return true;
        }

        void join(const std::string & id) {
            std::scoped_lock<std::mutex> mtx(threadMutex);

            // Check if thread exists
            if (threads.count(id) == 0) {
                return;
            }

            // Wait for thread to finish
            threads[id].join();
            threads.erase(id);
        }

        void sleepNano(const size_t ns) {
            svcSleepThread(ns);
        }

        void sleepMilli(const size_t ms) {
            sleepNano(ms * 1000000);
        }
    }
};
