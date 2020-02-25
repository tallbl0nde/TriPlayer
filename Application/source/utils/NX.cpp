#include "NX.hpp"

namespace Utils::NX {
    std::string getUserInput(unsigned int max, std::string ok, std::string hd, std::string subhd, std::string hint, std::string init) {
        SwkbdConfig kb;
        char * out = new char[max + 1];
        bool success = false;

        if (R_SUCCEEDED(swkbdCreate(&kb, 0))) {
            swkbdConfigMakePresetDefault(&kb);
            swkbdConfigSetStringLenMax(&kb, max);
            swkbdConfigSetReturnButtonFlag(&kb, 0);
            swkbdConfigSetOkButtonText(&kb, ok.c_str());
            swkbdConfigSetHeaderText(&kb, hd.c_str());
            swkbdConfigSetSubText(&kb, subhd.c_str());
            if (hint != "") {
                swkbdConfigSetGuideText(&kb, hint.c_str());
            }
            if (init != "") {
                swkbdConfigSetInitialText(&kb, init.c_str());
            }
            if (max <= 32) {
                swkbdConfigSetTextDrawType(&kb, SwkbdTextDrawType_Line);
            }
            swkbdShow(&kb, out, max + 1);
            swkbdClose(&kb);
            success = true;
        }

        std::string tmp(out);
        delete[] out;
        if (success) {
            return tmp;
        }
        return "";
    }
};