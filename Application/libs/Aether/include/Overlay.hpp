#ifndef AETHER_OVERLAY_HPP
#define AETHER_OVERLAY_HPP

#include "Screen.hpp"

namespace Aether {
    // An overlay is just a screen but with a transparent
    // black background.
    class Overlay : public Screen {
        private:
            // Set true when it should be closed
            bool close_;

        public:
            Overlay();

            // Call to mark the overlay to be closed
            void close(bool);
            // Returns close
            bool shouldClose();

            // Render calls Screen but draws background first
            void render();
    };
};

#endif