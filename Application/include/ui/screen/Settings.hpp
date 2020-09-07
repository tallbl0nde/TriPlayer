#ifndef SCREEN_SETTINGS_HPP
#define SCREEN_SETTINGS_HPP

#include "ui/element/SideButton.hpp"
#include "ui/screen/Screen.hpp"

namespace Main {
    class Application;
};

namespace Frame::Settings {
    class Frame;
};

namespace Screen {
    // Settings screen contains everything required to alter
    // either the app, overlay or sysmodule's config
    class Settings : public Screen {
        private:
            // Background elements
            Aether::Image * bg;
            Aether::Image * sidebarGradient;
            Aether::Rectangle * sidebarBg;

            // Containers
            Aether::List * sidebarList;
            Frame::Settings::Frame * frame;

            // Sidebar buttons
            CustomElm::SideButton * buttonAppGeneral;
            CustomElm::SideButton * buttonAppAppearance;
            CustomElm::SideButton * buttonAppSearch;
            CustomElm::SideButton * buttonAppAdvanced;
            CustomElm::SideButton * buttonSysGeneral;
            CustomElm::SideButton * buttonSysMP3;

            // Functions to set up each section
            void setupNew();

            void setupAppGeneral();
            void setupAppAppearance();
            void setupAppSearch();
            void setupAppAdvanced();

            void setupSysGeneral();
            void setupSysMP3();

            // Function called to go 'back'
            void backCallback();

        public:
            // Constructor takes pointer to app object
            Settings(Main::Application *);

            // Update colours
            void updateColours();

            // onLoad creates all shared elements
            void onLoad();

            // onUnload deletes created elements
            void onUnload();
    };
};

#endif