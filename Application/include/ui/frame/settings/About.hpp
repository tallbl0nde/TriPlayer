#ifndef FRAME_SETTINGS_ABOUT
#define FRAME_SETTINGS_ABOUT

#include "ui/frame/settings/Frame.hpp"

namespace Frame::Settings {
    // This frame contains information about TriPlayer, libraries, etc
    class About : public Frame {
        public:
            // Constructor creates needed elements
            About(Main::Application *);
    };
};

#endif