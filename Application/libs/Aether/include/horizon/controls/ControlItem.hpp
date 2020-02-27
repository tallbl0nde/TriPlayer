#ifndef AETHER_CONTROLITEM_HPP
#define AETHER_CONTROLITEM_HPP

#include "primary/Text.hpp"

namespace Aether {
    // A ControlItem is stored and handled by the "Controls" element.
    // It contains the two textures for a button icon and hint.
    class ControlItem : public Element {
        private:
            // Textures for button + hint
            Text * icon;
            Text * hint;

            // Colour for text
            Colour colour;

            // These functions can't be called
            using Element::addElement;
            using Element::removeElement;
            using Element::removeAllElements;
            using Element::setSelectable;

        public:
            // Constructor accepts button and text hint
            ControlItem(Button, std::string);

            // Return the set colour
            Colour getColour();
            // Set the colour
            void setColour(Colour);
            void setColour(uint8_t, uint8_t, uint8_t, uint8_t);
    };
};

#endif