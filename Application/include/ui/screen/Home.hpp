#ifndef SCREEN_HOME_HPP
#define SCREEN_HOME_HPP

#include "ui/element/Player.hpp"
#include "ui/element/SideButton.hpp"
#include "ui/overlay/AddToPlaylist.hpp"
#include "ui/frame/Frame.hpp"
#include "ui/screen/Screen.hpp"
#include <stack>

namespace Main {
    class Application;
};

namespace Screen {
    // Main screen (houses side menu, player and frames)
    class Home : public Screen {
        private:
            // Struct pair for a frame and next type
            struct FrameTuple {
                Frame::Frame * frame;       // Frame object
                Frame::Type type;      // Type of the frame stored in this pair
                Frame::Type pushedType;     // Type of next frame on stack (i.e. what pushed this frame on the stack?)
            };

            // This screen handles the Add to Playlist menu as it's seen on all frames
            CustomOvl::AddToPlaylist * addToPlMenu;

            // Misc
            Aether::Container * container;
            Aether::Image * bg;
            Aether::Image * sidegrad;
            Aether::Rectangle * playerDim;

            // Sidebar
            Aether::Image * backIcon;
            Aether::Text * backText;
            Aether::Element * backButton;
            Aether::Container * touchContainer;
            Aether::Container * sideContainer;
            Aether::Rectangle * sideBg;
            CustomElm::SideButton * sideSearch;
            Aether::Rectangle * sideSeparator;
            CustomElm::SideButton * sidePlaylists;
            CustomElm::SideButton * sideSongs;
            CustomElm::SideButton * sideArtists;
            CustomElm::SideButton * sideAlbums;
            Aether::Rectangle * sideSeparator2;
            CustomElm::SideButton * sideQueue;
            Aether::Rectangle * sideSeparator3;
            CustomElm::SideButton * sideSettings;

            // Player
            CustomElm::Player * player;

            // Whether to jump back some frames
            size_t backOneFrame;
            // Right-hand side 'frame'
            Frame::Frame * frame;
            Frame::Type frameType;
            // Stack of frames
            std::stack<FrameTuple> frameStack;

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

            // Shows 'Add to Playlist' menu and assigns given callback
            // Done here as every frame can call it with the same behaviour
            void showAddToPlaylist(std::function<void(PlaylistID)>);

            // Changes to the type of frame provided
            // Pass type, action to take and ID to pass to frame (not always used)
            void changeFrame(Frame::Type, Frame::Action, int = -1);

            // Update frame colours
            void updateColours();

            // Update player values
            void update(uint32_t);

            // onLoad creates all elements
            void onLoad();

            // onUnload deletes all elements
            void onUnload();
    };
};

#endif