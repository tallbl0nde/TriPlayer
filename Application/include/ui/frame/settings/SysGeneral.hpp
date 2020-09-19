#ifndef FRAME_SETTINGS_SYSGENERAL
#define FRAME_SETTINGS_SYSGENERAL

#include "ui/frame/settings/Frame.hpp"

namespace Frame::Settings {
    // Contains 'general' sysmodule settings
    class SysGeneral : public Frame {
        private:
            // Popuplist overlay
            Aether::PopupList * ovlList;

            // Helper to create popup
            void showLogLevelList(Aether::ListOption *);

        public:
            // Constructor sets up elements
            SysGeneral(Main::Application *);

            // Delete overlay
            ~SysGeneral();
    };
};

#endif