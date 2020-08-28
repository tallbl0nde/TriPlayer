#ifndef FRAME_SETTINGS_APPGENERAL
#define FRAME_SETTINGS_APPGENERAL

#include "ui/frame/settings/Frame.hpp"

namespace Frame::Settings {
    // This frame contains 'general' application settings
    class AppGeneral : public Frame {
        private:
            // 'PopupList' overlay
            Aether::PopupList * ovlList;

            // Show popup list for initial frame
            void showInitialFrameList(Aether::ListOption *);
            void showLogLevelList(Aether::ListOption *);

        public:
            AppGeneral(Main::Application *);

            // Delete any overlays
            ~AppGeneral();
    };
};

#endif