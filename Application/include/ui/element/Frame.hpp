#ifndef ELEMENT_FRAME_HPP
#define ELEMENT_FRAME_HPP

#include "Aether/Aether.hpp"

namespace Main {
    class Application;
};

namespace CustomElm {
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

        public:
            // Passed app pointer for sysmodule + theme
            Frame(Main::Application *);
    };
};

#endif