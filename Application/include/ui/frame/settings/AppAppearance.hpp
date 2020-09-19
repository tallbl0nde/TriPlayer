#ifndef FRAME_SETTINGS_APPEARANCE
#define FRAME_SETTINGS_APPEARANCE

#include "ui/frame/settings/Frame.hpp"

namespace Frame::Settings {
    // This frame contains 'appearance' related settings
    class AppAppearance : public Frame {
        private:
            // PopupList overlay
            Aether::PopupList * ovlList;

            // Helper to create list
            void showAccentColourList(Aether::ListOption *);

        public:
            // Constructor creates all elements
            AppAppearance(Main::Application *);

            // Delete overlay
            ~AppAppearance();
    };
};

#endif