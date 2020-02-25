#ifndef SCREEN_MAINSCREEN_HPP
#define SCREEN_MAINSCREEN_HPP

#include "Application.hpp"
#include "SideButton.hpp"

namespace Main {
    class Application;
};

namespace Screen {
    // Main screen
    class MainScreen : public Aether::Screen {
        private:
            Main::Application * app;

            // LHS
            // Icon
            // Aether::Image * icon;
            // Side list items
            CustomElm::SideButton * menuRP;
            CustomElm::SideButton * menuSongs;
            CustomElm::SideButton * menuArtists;
            CustomElm::SideButton * menuAlbums;
            CustomElm::SideButton * menuQueue;
            CustomElm::SideButton * menuSettings;

            // RHS
            // Heading
            Aether::Text * heading;
            // Search bar
            // CustomElm::SearchBox * search;
            // Column headings

            // List
            Aether::List * list;

            // BOTTOM

            // Deselect all side items
            void deselectSideItems();

        public:
            MainScreen(Main::Application *);

            // onLoad creates all elements
            void onLoad();

            // onUnload deletes all elements
            void onUnload();

            // Redraw rectangles every frame to save some memory
            void render();
    };
};

#endif