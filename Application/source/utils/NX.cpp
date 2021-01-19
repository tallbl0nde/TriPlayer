#include "lang/Language.hpp"
#include "Log.hpp"
#include <switch.h>
#include "utils/NX.hpp"

namespace Utils::NX {
    void startServices() {
        pmdmntInitialize();
        pmshellInitialize();
        romfsInit();
        socketInitializeDefault();
    }

    void stopServices() {
        pmdmntExit();
        pmshellExit();
        romfsExit();
        socketExit();
    }

    Language getSystemLanguage() {
        // Read from settings
        SetLanguage sl;
        u64 l;
        setInitialize();
        setGetSystemLanguage(&l);
        setMakeLanguage(l, &sl);
        setExit();

        // Return appropriate language enum
        Language lang;
        switch (sl) {
            case SetLanguage_ENGB:
            case SetLanguage_ENUS:
                lang = Language::English;
                break;

            // No translations for these yet
            case SetLanguage_FR:
            case SetLanguage_FRCA:
            case SetLanguage_DE:
            case SetLanguage_ES:
            case SetLanguage_IT:
            case SetLanguage_NL:
            case SetLanguage_PT:
            case SetLanguage_JA:
            case SetLanguage_RU:
            case SetLanguage_ZHHANT:
            case SetLanguage_ZHCN:
            case SetLanguage_ZHHANS:
            case SetLanguage_ZHTW:
            case SetLanguage_KO:
            case SetLanguage_ES419:
            default:
                lang = Language::Default;
                break;
        }

        return lang;
    }

    bool getUserInput(Keyboard & k) {
        SwkbdConfig kb;
        char * out = new char[k.maxLength + 1];
        bool success = false;

        // Create initial object
        if (R_SUCCEEDED(swkbdCreate(&kb, 0))) {
            swkbdConfigMakePresetDefault(&kb);
            swkbdConfigSetType(&kb, SwkbdType_All);
            swkbdConfigSetDicFlag(&kb, 1);
            swkbdConfigSetInitialText(&kb, k.buffer.c_str());
            swkbdConfigSetOkButtonText(&kb, k.ok.c_str());
            swkbdConfigSetReturnButtonFlag(&kb, 0);
            swkbdConfigSetStringLenMax(&kb, (k.showLine ? (k.maxLength > 32 ? 32 : k.maxLength) : k.maxLength));

            // Set config to show line
            if (k.showLine) {
                swkbdConfigSetTextDrawType(&kb, SwkbdTextDrawType_Line);
                swkbdConfigSetHeaderText(&kb, k.heading.c_str());
                swkbdConfigSetSubText(&kb, k.subHeading.c_str());

            // Set config to show box
            } else {
                swkbdConfigSetTextDrawType(&kb, SwkbdTextDrawType_Box);
                swkbdConfigSetGuideText(&kb, k.hint.c_str());
            }

            // Show keyboard
            if (R_SUCCEEDED(swkbdShow(&kb, out, k.maxLength + 1))) {
                success = true;
            }
            swkbdClose(&kb);
        }

        if (success) {
            k.buffer = std::string(out);
        }
        delete[] out;
        return success;
    }

    bool getUserInput(Numpad & n) {
        if (n.maxDigits > 32) {
            n.maxDigits = 32;
        }

        SwkbdConfig kb;
        char * out = new char[n.maxDigits + 1];
        bool success = false;

        // Create initial object
        if (R_SUCCEEDED(swkbdCreate(&kb, 0))) {
            swkbdConfigSetType(&kb, SwkbdType_NumPad);
            swkbdConfigSetInitialText(&kb, std::to_string(n.value).c_str());
            swkbdConfigSetInitialCursorPos(&kb, 1);
            swkbdConfigSetBlurBackground(&kb, 1);
            swkbdConfigSetHeaderText(&kb, n.heading.c_str());
            swkbdConfigSetSubText(&kb, n.subHeading.c_str());
            swkbdConfigSetStringLenMax(&kb, n.maxDigits);
            swkbdConfigSetTextDrawType(&kb, SwkbdTextDrawType_Line);
            if (n.allowNegative) {
                swkbdConfigSetLeftOptionalSymbolKey(&kb, "-");
            }
            if (n.allowDecimal) {
                swkbdConfigSetRightOptionalSymbolKey(&kb, ".");
            }

            if (R_SUCCEEDED(swkbdShow(&kb, out, n.maxDigits))) {
                success = true;
            }
            swkbdClose(&kb);
        }

        if (success) {
            n.value = std::strtol(out, nullptr, 10);
        }

        delete[] out;
        return success;
    }

    static bool boost = false;
    void setCPUBoost(bool enable) {
        // Only set if different state
        if (enable == boost) {
            return;
        }

        appletSetCpuBoostMode((enable ? ApmCpuBoostMode_FastLoad : ApmCpuBoostMode_Normal));
        boost = enable;
    }

    void setLowFsPriority(bool low) {
        fsSetPriority(low ? FsPriority_Background : FsPriority_Normal);
    }

    static bool media = false;
    void setPlayingMedia(bool enable) {
        // Only set if different state
        if (enable == media) {
            return;
        }

        appletSetMediaPlaybackState(enable);
        media = enable;
    }

    bool launchProgram(unsigned long long programID) {
        // Create struct to pass
        NcmProgramLocation location;
        location.program_id = programID;
        location.storageID = NcmStorageId_None;

        // Check if running
        if (runningProgram(programID)) {
            Log::writeError("[NX] Can't launch program as it's running!");
            return false;
        }

        // Attempt to launch
        u64 pid;
        Result rc = pmshellLaunchProgram(PmLaunchFlag_None, &location, &pid);
        if (R_FAILED(rc)) {
            Log::writeError("[NX] Failed to launch program with ID: " + std::to_string(programID));
            return false;

        } else {
            Log::writeInfo("[NX] Launched program with ID: " + std::to_string(programID) + ", it has pid: " + std::to_string(pid));
            return true;
        }
    }

    bool runningProgram(unsigned long long programID) {
        u64 pid;
        Result rc = pmdmntGetProcessId(&pid, programID);
        if (R_SUCCEEDED(rc)) {
            Log::writeInfo("[NX] Program with ID: " + std::to_string(programID) + " is running with pid " + std::to_string(pid));
            return true;
        }
        Log::writeInfo("[NX] Program with ID: " + std::to_string(programID) + " is not running");
        return false;
    }

    bool terminateProgram(unsigned long long programID) {
        Result rc = pmshellTerminateProgram(programID);
        if (R_FAILED(rc)) {
            Log::writeError("[NX] Failed to terminate program with ID: " + std::to_string(programID));
            return false;
        }
        return true;
    }
};