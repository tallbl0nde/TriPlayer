#ifndef AETHER_EVENT_HPP
#define AETHER_EVENT_HPP

#include "utils/Types.hpp"

namespace Aether {
    // Supported types of events
    enum EventType {
        ButtonPressed,
        ButtonReleased,
        TouchPressed,
        TouchMoved,
        TouchReleased
    };

    // An InputEvent is basically an SDL_Event but minus the dependency
    // on SDL, it also removes all unnecessary events that aren't handled
    // by Aether.
    class InputEvent {
        private:
            // Type of event
            EventType type_;
            // Button info (don't read when not button event!)
            Button button_;
            int id_;
            // Touch info (don't read when not touch event!)
            // Coordinates are from (0 - 1280) and (0 - 720)
            int touchX_;
            int touchY_;
            int touchDX_;
            int touchDY_;

        public:
            // Constructors takes SDL_Event and "converts" to InputEvent
            InputEvent(SDL_Event);

            // Getters for members
            EventType type();
            Button button();
            int id();
            int touchX();
            int touchY();
            int touchDX();
            int touchDY();

            // Destructor does nothing
            ~InputEvent();
    };
};

#endif