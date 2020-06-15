#ifndef SCREEN_MAINSCREEN_HPP
#define SCREEN_MAINSCREEN_HPP

#include "Frame.hpp"
#include "Player.hpp"
#include "SearchBox.hpp"
#include "SideButton.hpp"

namespace Main {
    class Application;
};

namespace Screen {
    // Main screen
    class MainScreen : public Aether::Screen {
        private:
            Main::Application * app;

            // Misc
            Aether::Image * bg;
            Aether::Image * sidegrad;

            // Sidebar
            Aether::Rectangle * sideBg;
            Aether::Rectangle * userBg;
            Aether::Image * userIcon;
            Aether::Text * userText;
            Aether::Rectangle * settingsBg;
            Aether::Image * settingsIcon;
            CustomElm::SearchBox * search;
            CustomElm::SideButton * sideRP;
            CustomElm::SideButton * sideSongs;
            CustomElm::SideButton * sideArtists;
            CustomElm::SideButton * sideAlbums;
            Aether::Rectangle * sideSeparator;
            CustomElm::SideButton * sideQueue;

            // Player
            CustomElm::Player * player;

            // Right-hand side 'frame'
            CustomElm::Frame * frame;

            // Cached vars to avoid updating every frame
            SongID playingID;

            // 'Reset' screen state - deselect side items, and delete frame
            void resetState();

            // Functions to setup screen based on chosen item
            void setupQueue();
            void setupSongs();

        public:
            MainScreen(Main::Application *);

            void update(uint32_t);

            // onLoad creates all elements
            void onLoad();

            // onUnload deletes all elements
            void onUnload();
    };
};

#endif