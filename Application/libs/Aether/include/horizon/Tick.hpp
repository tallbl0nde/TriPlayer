#ifndef AETHER_TICK_HPP
#define AETHER_TICK_HPP

#include "primary/Ellipse.hpp"
#include "primary/Text.hpp"

namespace Aether {
    // A Tick is an element that looks the same as the tick shown
    // in Horizon - a coloured circle with a tick on top
    class Tick : public Element {
        private:
            // Pointers to two elements
            Ellipse * circle;
            Text * tick;

        public:
            // Takes x, y, diameter
            Tick(int, int, unsigned int);

            // Getter + setters
            unsigned int size();
            void setSize(unsigned int);
            Colour getCircleColour();
            void setCircleColour(Colour);
            Colour getTickColour();
            void setTickColour(Colour);

            // Render adjusts blend mode for text
            void render();
    };
};

#endif