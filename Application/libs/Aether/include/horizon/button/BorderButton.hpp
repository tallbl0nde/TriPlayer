#ifndef AETHER_BORDERBUTTON_HPP
#define AETHER_BORDERBUTTON_HPP

#include "base/Element.hpp"
#include "primary/Box.hpp"
#include "primary/Text.hpp"

namespace Aether {
    // A BorderButton is exactly that. The text/box are resized/repositioned to be centered
    // on any resizes.
    class BorderButton : public Element {
        private:
            // Pointers to elements
            Box * box;
            Text * text;

        public:
            // X, Y, W, H, Border size, Text, Text size and Callback
            BorderButton(int, int, int, int, unsigned int, std::string, unsigned int, std::function<void()>);

            // Getter + setter for colours
            Colour getBorderColour();
            void setBorderColour(Colour);
            Colour getTextColour();
            void setTextColour(Colour);

            // Getter + setter for string
            std::string getString();
            void setString(std::string);

            // Adjusting dimensions requires text pos to be adjusted
            // And box to be rendered again
            void setW(int);
            void setH(int);

            // Rendering the highlight needs to be rounded (and overlapping)
            void render();
            void renderHighlighted();
            void renderSelected();
    };
};

#endif