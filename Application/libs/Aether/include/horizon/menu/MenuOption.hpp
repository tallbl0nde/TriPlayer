#ifndef AETHER_MENUOPTION_HPP
#define AETHER_MENUOPTION_HPP

#include "base/Element.hpp"
#include "primary/Rectangle.hpp"
#include "primary/Text.hpp"

namespace Aether {
    // A MenuOption is a combination of elements which looks like and functions
    // similar to Horizon's. All children are handled internally
    class MenuOption : public Element {
        private:
            // Draws active style when true
            bool active;

            // Colours to tint when active or not
            Colour activeColour;
            Colour inactiveColour;

            // Child element pointers (required to update pointers)
            Rectangle * rect;
            Text * text;

        public:
            // Constructor takes string, active/inactive colours and callback function
            MenuOption(std::string, Colour, Colour, std::function<void()>);

            // Setting the width needs to adjust width of text texture
            void setW(int);

            // Setter for active
            // Changes look of option based on bool
            void setActive(bool);

            // Setter for colours (adjusts children colours)
            void setActiveColour(Colour);
            void setInactiveColour(Colour);
    };
};

#endif