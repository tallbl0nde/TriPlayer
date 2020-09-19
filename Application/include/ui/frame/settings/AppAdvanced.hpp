#ifndef FRAME_SETTINGS_APPADVANCED
#define FRAME_SETTINGS_APPADVANCED

#include "ui/frame/settings/Frame.hpp"

namespace Frame::Settings {
    // This frame contains 'advanced' settings (who woulda guessed?)
    class AppAdvanced : public Frame {
        private:
            // Helper called by 'Remove Images' to clean up database and folder
            void removeImages();

        public:
            // Constructor creates needed elements
            AppAdvanced(Main::Application *);
    };
};

#endif