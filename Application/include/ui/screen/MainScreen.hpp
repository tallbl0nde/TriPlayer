#ifndef SCREEN_MAINSCREEN_HPP
#define SCREEN_MAINSCREEN_HPP

#include "ui/element/Frame.hpp"
#include "ui/element/Player.hpp"
#include "ui/element/SearchBox.hpp"
#include "ui/element/SideButton.hpp"

namespace Main {
    class Application;
};

namespace Screen {
    // Main screen
    class MainScreen : public Aether::Screen {
        private:
            Main::Application * app;

            // Misc
            Aether::Container * container;
            Aether::Image * bg;
            Aether::Image * sidegrad;
            Aether::Rectangle * playerDim;

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

            // Finalize screen state - add elements, set frame
            void finalizeState();
            // 'Reset' screen state - deselect side items, and delete frame
            void resetState();

            // Functions to setup screen based on chosen item
            void setupArtists();
            void setupQueue();
            void setupSongs();

        public:
            MainScreen(Main::Application *);

            // Update player values
            void update(uint32_t);

            // onLoad creates all elements
            void onLoad();

            // onUnload deletes all elements
            void onUnload();
    };
};

#endif