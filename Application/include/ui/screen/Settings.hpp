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
            CustomElm::SideButton * buttonBack;
            CustomElm::SideButton * buttonAppGeneral;
            CustomElm::SideButton * buttonAppAppearance;
            CustomElm::SideButton * buttonAppMetadata;
            CustomElm::SideButton * buttonAppSearch;
            CustomElm::SideButton * buttonAppAdvanced;
            CustomElm::SideButton * buttonSysGeneral;
            CustomElm::SideButton * buttonSysMP3;
            CustomElm::SideButton * buttonAbout;
            Aether::Ellipse * updateDot;

            // Functions to set up each section
            void setupNew();

            void setupAppGeneral();
            void setupAppAppearance();
            void setupAppMetadata();
            void setupAppSearch();
            void setupAppAdvanced();

            void setupSysGeneral();
            void setupSysMP3();

            void setupAbout();

            // Function called to go 'back'
            void backCallback();

            // Helpers to create list element
            void addHeading(const std::string &);

        public:
            // Constructor takes pointer to app object
            Settings(Main::Application *);

            // Update colours
            void updateColours();

            // Constantly checks if update is available
            void update(uint32_t);

            // onLoad creates all shared elements
            void onLoad();

            // onUnload deletes created elements
            void onUnload();
    };
};

#endif