#ifndef AETHER_LISTHEADING_HPP
#define AETHER_LISTHEADING_HPP

#include "base/Element.hpp"
#include "primary/Rectangle.hpp"
#include "primary/Text.hpp"

namespace Aether {
    // A listHeading is a small rectangle with text alongside, usually used in a list.
    class ListHeading : public Element {
        private:
            Rectangle * rect;
            Text * text;

        public:
            // Constructor creates rect + text
            // String
            ListHeading(std::string);

            // Getter + setter for colours
            Colour getRectColour();
            void setRectColour(Colour);
            Colour getTextColour();
            void setTextColour(Colour);
    };
};

#endif