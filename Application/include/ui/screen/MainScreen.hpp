#ifndef SCREEN_MAINSCREEN_HPP
#define SCREEN_MAINSCREEN_HPP

#include <stack>
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
            // Stack of frames
            std::stack<CustomElm::Frame *> frameStack;

            // Cached vars to avoid updating every frame
            SongID playingID;

            // Finalize screen state - add elements, set frame
            void finalizeState();
            // Undoes finalizeState()
            void resetState();

        public:
            MainScreen(Main::Application *);

            // Changes to the type of frame provided
            // Pass type, action to take and ID to pass to frame (not always used)
            void changeFrame(Frame::Type, Frame::Action, int = -1);

            // Update player values
            void update(uint32_t);

            // onLoad creates all elements
            void onLoad();

            // onUnload deletes all elements
            void onUnload();
    };
};

#endif