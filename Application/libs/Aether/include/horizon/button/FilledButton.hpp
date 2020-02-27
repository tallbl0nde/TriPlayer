#ifndef AETHER_FILLEDBUTTON_HPP
#define AETHER_FILLEDBUTTON_HPP

#include "base/Element.hpp"
#include "primary/Rectangle.hpp"
#include "primary/Text.hpp"

namespace Aether {
    // A FilledButton is exactly that. The text/rectangle are resized/repositioned to be centered
    // on any resizes.
    class FilledButton : public Element {
        private:
            // Pointers to elements
            Rectangle * rect;
            Text * text;

        public:
            // X, Y, W, H, Text, Text size and Callback
            FilledButton(int, int, int, int, std::string, unsigned int, std::function<void()>);

            // Getter + setter for colours
            Colour getFillColour();
            void setFillColour(Colour);
            Colour getTextColour();
            void setTextColour(Colour);

            // Getter + setter for string
            std::string getString();
            void setString(std::string);

            // Adjusting dimensions requires text pos to be adjusted
            // And rectangle to be rendered again
            void setW(int);
            void setH(int);

            // Rendering the highlight needs to be rounded
            void renderHighlighted();
            void renderSelected();
    };
};

#endif