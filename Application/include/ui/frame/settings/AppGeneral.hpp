#ifndef FRAME_SETTINGS_APPGENERAL
#define FRAME_SETTINGS_APPGENERAL

#include "ui/frame/settings/Frame.hpp"

namespace Frame::Settings {
    // This frame contains 'general' application settings
    class AppGeneral : public Frame {
        public:
            AppGeneral(Main::Application *);
    };
};

#endif