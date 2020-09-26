#include "Log.hpp"
#include <switch.h>
#include "utils/NX.hpp"

namespace Utils::NX {
    void startServices() {
        pmshellInitialize();
        romfsInit();
    }

    void stopServices() {
        pmshellExit();
        romfsExit();
    }

    bool getUserInput(Keyboard & k) {
        SwkbdConfig kb;
        char * out = new char[k.maxLength + 1];
        bool success = false;

        // Create initial object
        if (R_SUCCEEDED(swkbdCreate(&kb, 0))) {
            swkbdConfigMakePresetDefault(&kb);
            swkbdConfigSetType(&kb, SwkbdType_Normal);
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

        appletSetCpuBoostMode((enable ? ApmCpuBoostMode_Type1 : ApmCpuBoostMode_Disabled));
        boost = enable;
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

    bool terminateProgram(unsigned long long programID) {
        Result rc = pmshellTerminateProgram(programID);
        if (R_FAILED(rc)) {
            Log::writeError("[NX] Failed to terminate program with ID: " + std::to_string(programID));
            return false;
        }
        return true;
    }
};