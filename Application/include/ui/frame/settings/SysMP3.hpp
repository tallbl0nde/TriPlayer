#ifndef FRAME_SETTINGS_SYSMP3
#define FRAME_SETTINGS_SYSMP3

#include "ui/frame/settings/Frame.hpp"
#include "ui/overlay/Equalizer.hpp"

namespace Frame::Settings {
    // Contains sysmodule settings for mp3 files
    class SysMP3 : public Frame {
        private:
            // Equalizer overlay
            CustomOvl::Equalizer * ovlEQ;

            // Helper to set up overlay
            void showEqualizer();

        public:
            // Constructor creates all elements
            SysMP3(Main::Application *);

            // Delete overlay
            ~SysMP3();
    };
};

#endif