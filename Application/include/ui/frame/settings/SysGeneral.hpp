#ifndef FRAME_SETTINGS_SYSGENERAL
#define FRAME_SETTINGS_SYSGENERAL

#include "ui/frame/settings/Frame.hpp"
#include "ui/overlay/ComboPicker.hpp"

namespace Frame::Settings {
    // Contains 'general' sysmodule settings
    class SysGeneral : public Frame {
        private:
            // Popuplist overlay
            Aether::PopupList * ovlList;
            // Helper to create popup
            void showLogLevelList(Aether::ListOption *);

            // Button combination picker
            CustomOvl::ComboPicker * ovlCombo;
            // Helper to create button popup
            void showPickCombo(const std::string &, Aether::ListOption *, std::function<std::vector<NX::Button>()>, std::function<bool(std::vector<NX::Button>)>);

        public:
            // Constructor sets up elements
            SysGeneral(Main::Application *);

            // Delete overlay
            ~SysGeneral();
    };
};

#endif