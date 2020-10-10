#include <cstring>
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
    static bool socketInitialized = false;

    // Starts all needed services
    bool startServices() {
        // Prevent starting twice
        if (audrenInitialized || fsInitialized || gpioInitialized || hidInitialized || pscmInitialized || smInitialized || socketInitialized) {
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

        // Sockets (uses small buffers)
        constexpr SocketInitConfig socketCfg = {
            .bsdsockets_version = 1,

            .tcp_tx_buf_size = 0x1000,
            .tcp_rx_buf_size = 0x1000,
            .tcp_tx_buf_max_size = 0x3000,
            .tcp_rx_buf_max_size = 0x3000,

            .udp_tx_buf_size = 0x0,
            .udp_rx_buf_size = 0x0,

            .sb_efficiency = 1,
        };
        rc = socketInitialize(&socketCfg);
        if (R_SUCCEEDED(rc)) {
            socketInitialized = true;

        } else {
            logError("socket", rc);
            return false;
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

        // Sockets
        if (socketInitialized) {
            socketExit();
            socketInitialized = false;
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
        constexpr char delims[] = " +";     // Supported delimiters

        std::vector<Button> stringToCombo(const std::string & str) {
            // Vector to return
            std::vector<Button> combo;

            // Ensure we have a string
            if (str.length() > 0) {
                // Split on delimiters
                std::vector<std::string> tokens;
                char * copy = strdup(str.c_str());
                char * tok = std::strtok(copy, delims);
                while (tok != nullptr) {
                    tokens.push_back(std::string(tok));
                    tok = std::strtok(nullptr, delims);
                }
                free(copy);

                // Convert each word to upper case (only handles ASCII but that's all we support)
                for (std::string & token : tokens) {
                    std::transform(token.begin(), token.end(), token.begin(), [](unsigned char c) {
                        return std::toupper(c);
                    });
                }

                // Finally compare each token to see if it matches a button
                // Note that we return an empty vector even if only one button is wrong
                for (std::string & token : tokens) {
                    if (token == "A") {
                        combo.push_back(Button::A);

                    } else if (token == "B") {
                        combo.push_back(Button::B);

                    } else if (token == "X") {
                        combo.push_back(Button::X);

                    } else if (token == "Y") {
                        combo.push_back(Button::Y);

                    } else if (token == "LSTICK") {
                        combo.push_back(Button::LSTICK);

                    } else if (token == "RSTICK") {
                        combo.push_back(Button::RSTICK);

                    } else if (token == "L") {
                        combo.push_back(Button::L);

                    } else if (token == "R") {
                        combo.push_back(Button::R);

                    } else if (token == "ZL") {
                        combo.push_back(Button::ZL);

                    } else if (token == "ZR") {
                        combo.push_back(Button::ZR);

                    } else if (token == "PLUS") {
                        combo.push_back(Button::PLUS);

                    } else if (token == "MINUS") {
                        combo.push_back(Button::MINUS);

                    } else if (token == "DLEFT") {
                        combo.push_back(Button::DLEFT);

                    } else if (token == "DUP") {
                        combo.push_back(Button::DUP);

                    } else if (token == "DRIGHT") {
                        combo.push_back(Button::DRIGHT);

                    } else if (token == "DDOWN") {
                        combo.push_back(Button::DDOWN);

                    } else {
                        combo.clear();
                        break;
                    }
                }
            }

            return combo;
        }

        bool comboPressed(const std::vector<Button> & buttons) {
            // Scan input first
            hidScanInput();
            uint64_t pressed = hidKeysHeld(CONTROLLER_P1_AUTO);

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
        constexpr u16 pscDependencies[] = {PscPmModuleId_Audio};    // Our dependencies
        constexpr PscPmModuleId pscModuleId = (PscPmModuleId)690;   // Our ID
        static PscPmModule pscModule;                               // Module to listen for events with
        static bool pscPrepared = false;                            // Set true if ready to handle event

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
            Result rc = pscmGetPmModule(&pscModule, pscModuleId, pscDependencies, sizeof(pscDependencies)/sizeof(u16), true);
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

        bool enteringSleep(const size_t ms) {
            size_t ns = 1000000 * ms;

            // Don't wait for event if not prepared
            if (!pscPrepared) {
                svcSleepThread(ns);
                return false;
            }

            // Wait for an event
            Result rc = eventWait(&pscModule.event, ns);
            if (R_VALUE(rc) == KERNELRESULT(TimedOut)) {
                return false;

            } else if (R_VALUE(rc) == KERNELRESULT(Cancelled)) {
                svcSleepThread(ns);
                return false;
            }

            // If there's an event, fetch the state
            PscPmState eventState;
            u32 flags;
            rc = pscPmModuleGetRequest(&pscModule, &eventState, &flags);
            if (R_FAILED(rc)) {
                return false;
            }

            // Check if we're entering sleep
            bool enteringSleep = false;
            if (eventState == PscPmState_ReadySleep) {
                enteringSleep = true;
            }
            pscPmModuleAcknowledge(&pscModule, eventState);
            return enteringSleep;
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