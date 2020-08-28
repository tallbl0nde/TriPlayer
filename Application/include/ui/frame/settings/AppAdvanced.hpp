#ifndef FRAME_SETTINGS_APPADVANCED
#define FRAME_SETTINGS_APPADVANCED

#include "ui/frame/settings/Frame.hpp"

namespace Frame::Settings {
    // This frame contains 'advanced' settings (who woulda guessed?)
    class AppAdvanced : public Frame {
        public:
            // Constructor creates needed elements
            AppAdvanced(Main::Application *);
    };
};

#endif