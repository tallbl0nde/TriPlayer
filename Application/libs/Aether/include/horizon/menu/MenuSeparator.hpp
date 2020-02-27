#ifndef AETHER_MENUSEPERATOR_HPP
#define AETHER_MENUSEPARATOR_HPP

#include "base/Element.hpp"
#include "primary/Rectangle.hpp"

namespace Aether {
    // A MenuSeparator contains a rectangle with some padding
    // above/below. It is not selectable.
    class MenuSeparator : public Element {
        private:
            // Rectangle object to draw
            Rectangle * rect;

        public:
            // Constructor creates rectangle with width 100px
            MenuSeparator(Colour = {255, 255, 255, 255});

            // Adjusting width needs to rescale rectangle
            void setW(int);
    };
};

#endif