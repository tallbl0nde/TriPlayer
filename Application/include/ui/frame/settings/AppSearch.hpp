#ifndef FRAME_SETTINGS_APPSEARCH
#define FRAME_SETTINGS_APPSEARCH

#include "ui/frame/settings/Frame.hpp"

namespace Frame::Settings {
    // This frame contains search related settings
    class AppSearch : public Frame {
        public:
            // Constructor sets up elements
            AppSearch(Main::Application *);
    };
};

#endif