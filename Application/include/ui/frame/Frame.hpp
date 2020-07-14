#ifndef FRAME_FRAME_HPP
#define FRAME_FRAME_HPP

#include "Aether/Aether.hpp"

namespace Main {
    class Application;
};

namespace Frame {
    // Action to take on setting frame
    enum class Action {
        Push,       // Push the current frame onto the stack
        Reset       // Empty the stack (deleting any frames on the stack)
    };

    // Different types of frames (used for changing to another frame from within a frame)
    enum class Type {
        Playlists,
        Playlist,
        Albums,
        Album,
        Artists,
        Artist,
        Songs,
        Queue
    };

    // A frame is a container that is 'swapped out' on the right hand side
    // of the main screen. The constructor prepares all elements.
    class Frame : public Aether::Container {
        protected:
            // Common elements
            Aether::Text * heading;
            Aether::Text * subLength;
            Aether::Text * subTotal;
            Aether::Text * titleH;
            Aether::Text * artistH;
            Aether::Text * albumH;
            Aether::Text * lengthH;
            Aether::List * list;

            // Pointer to app to access shared objects
            Main::Application * app;
            // Pointer to function set below
            std::function<void(Type, Action, int)> changeFrame;

        public:
            // Passed app pointer for sysmodule + theme
            Frame(Main::Application *);

            // Passed a function to call when wanting to add a frame
            void setChangeFrameFunc(std::function<void(Type, Action, int)>);
    };
};

#endif