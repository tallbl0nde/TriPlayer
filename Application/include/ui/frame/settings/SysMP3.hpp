#ifndef FRAME_SETTINGS_SYSMP3
#define FRAME_SETTINGS_SYSMP3

#include "ui/frame/settings/Frame.hpp"

namespace Frame::Settings {
    // Contains sysmodule settings for mp3 files
    class SysMP3 : public Frame {
        public:
            // Constructor creates all elements
            SysMP3(Main::Application *);
    };
};

#endif