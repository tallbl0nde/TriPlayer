#ifndef SCREEN_HOME_HPP
#define SCREEN_HOME_HPP

#include "ui/element/Player.hpp"
#include "ui/element/SearchBox.hpp"
#include "ui/element/SideButton.hpp"
#include "ui/frame/Frame.hpp"
#include <stack>

namespace Main {
    class Application;
};

namespace Screen {
    // Main screen (houses side menu, player and frames)
    class Home : public Aether::Screen {
        private:
            Main::Application * app;

            // Misc
            Aether::Container * container;
            Aether::Image * bg;
            Aether::Image * sidegrad;
            Aether::Rectangle * playerDim;

            // Sidebar
            Aether::Image * backIcon;
            Aether::Text * backText;
            Aether::Element * backButton;
            Aether::Container * sideContainer;
            Aether::Rectangle * sideBg;
            Aether::Rectangle * userBg;
            Aether::Image * userIcon;
            Aether::Text * userText;
            Aether::Rectangle * settingsBg;
            Aether::Image * settingsIcon;
            CustomElm::SearchBox * search;
            CustomElm::SideButton * sidePlaylists;
            CustomElm::SideButton * sideSongs;
            CustomElm::SideButton * sideArtists;
            CustomElm::SideButton * sideAlbums;
            Aether::Rectangle * sideSeparator;
            CustomElm::SideButton * sideQueue;

            // Player
            CustomElm::Player * player;

            // Whether to jump back some frames
            size_t backOneFrame;
            // Right-hand side 'frame'
            Frame::Frame * frame;
            // Stack of frames
            std::stack<Frame::Frame *> frameStack;

            // Cached vars to avoid updating every frame
            SongID playingID;

            // Function called to go 'back'
            void backCallback();

            // Finalize screen state - add elements, set frame
            void finalizeState();
            // Undoes finalizeState()
            void resetState();

        public:
            Home(Main::Application *);

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