#ifndef SCREEN_MAINSCREEN_HPP
#define SCREEN_MAINSCREEN_HPP

#include "Application.hpp"
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
            Aether::Rectangle * playerBg;
            Aether::Image * albumCover;
            Aether::Text * trackName;
            Aether::Text * trackArtist;
            Aether::Image * shuffle;
            Aether::Image * previous;
            Aether::Image * play;
            Aether::Image * pause;
            Aether::Image * next;
            Aether::Image * repeat;
            Aether::Text * position;
            Aether::RoundProgressBar * seekBar;
            Aether::Text * duration;
            Aether::Image * volumeIcon;
            Aether::RoundProgressBar * volume;
            Aether::Image * fullscreen;

            // Main 'section'
            Aether::Text * heading;
            Aether::Text * titleH;
            Aether::Text * artistH;
            Aether::Text * albumH;
            Aether::Text * lengthH;
            Aether::List * list;

            // Deselect all side items
            void deselectSideItems();

            // Functions to setup screen based on chosen item
            void setupSongs();
            // and so on...

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