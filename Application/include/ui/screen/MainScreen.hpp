#ifndef SCREEN_MAINSCREEN_HPP
#define SCREEN_MAINSCREEN_HPP

#include "Frame.hpp"
#include "SearchBox.hpp"
#include "SideButton.hpp"
#include "Slider.hpp"

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
            Aether::Image * repeatOne;
            Aether::Text * position;
            CustomElm::Slider * seekBar;
            Aether::Text * duration;
            Aether::Image * volumeIcon;
            CustomElm::Slider * volume;
            Aether::Image * fullscreen;

            // Right-hand side 'frame'
            CustomElm::Frame * frame;

            // Cached vars to avoid updating every frame
            SongID playingID;
            RepeatMode repeatMode;
            ShuffleMode shuffleMode;

            // Duration of playing song in seconds
            unsigned int playingDuration;

            // 'Reset' screen state - deselect side items, and delete frame
            void resetState();

            // Functions to setup screen based on chosen item
            void setupQueue();
            void setupSongs();

            // "Helpers"
            void setRepeatIcon();

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