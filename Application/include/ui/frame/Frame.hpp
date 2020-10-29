#ifndef FRAME_FRAME_HPP
#define FRAME_FRAME_HPP

#include "Aether/Aether.hpp"
#include "Types.hpp"

namespace Main {
    class Application;
};

namespace Frame {
    // Action to take on setting frame
    enum class Action {
        Back,       // Pop and delete the current frame and go back to the previous one
        Push,       // Push the current frame onto the stack
        Reset       // Empty the stack (deleting any frames on the stack)
    };

    // Different types of frames (used for changing to another frame from within a frame)
    enum class Type {
        Playlists,
        Playlist,
        PlaylistInfo,
        Albums,
        Album,
        AlbumInfo,
        Artists,
        Artist,
        ArtistInfo,
        Search,
        Songs,
        SongInfo,
        Queue,
        None            // Only passed in special cases (e.g. when action is ::Reset)
    };

    // A frame is a container that is 'swapped out' on the right hand side
    // of the main screen. The constructor prepares all elements.
    class Frame : public Aether::Container {
        protected:
            // Common elements
            Aether::Container * topContainer;
            Aether::Text * heading;
            Aether::Text * subHeading;
            Aether::BorderButton * sort;
            Aether::Text * titleH;
            Aether::Text * artistH;
            Aether::Text * albumH;
            Aether::Text * lengthH;
            Aether::Container * bottomContainer;
            Aether::List * list;

            // Pointer to app to access shared objects
            Main::Application * app;
            // Pointers to callbacks set below
            std::function<void(Type, Action, int)> changeFrame;
            std::function<void(const std::string &, const std::vector<SongID> &, const size_t, const bool)> playNewQueue;
            std::function<void(std::function<void(PlaylistID)>)> showAddToPlaylist;

        public:
            // Passed app pointer for sysmodule + theme
            Frame(Main::Application *);

            // Listens for specific button presses
            bool handleEvent(Aether::InputEvent *);

            // Call to update colours
            virtual void updateColours();

            // Called when the frame is pushed onto the screen's stack
            // Passed next frame type
            virtual void onPush(Type);
            // Called when the frame is popped the screen's stack (even when Reset!)
            // Passed previous frame type
            virtual void onPop(Type);

            // Passed a function to call when wanting to add a frame
            void setChangeFrameFunc(std::function<void(Type, Action, int)>);
            // Passed a function to call when wanting to set a new play queue
            void setPlayNewQueueFunc(std::function<void(const std::string &, const std::vector<SongID> &, const size_t, const bool)>);
            // Passed a function to call when wanting to add to playlist
            void setShowAddToPlaylistFunc(std::function<void(std::function<void(PlaylistID)>)>);
    };
};

#endif