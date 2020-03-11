#ifndef ELEMENT_SEARCHBOX_HPP
#define ELEMENT_SEARCHBOX_HPP

#include "Aether.hpp"

namespace CustomElm {
    class SearchBox : public Aether::Element {
        private:
            // Round rect
            Aether::Rectangle * rect;

            // Search icon
            Aether::Image * icon;

            // Search text
            Aether::Text * text;

        public:
            // x, y, w
            SearchBox(int, int, int);

            // Set colours
            void setBoxColour(Aether::Colour);
            void setIconColour(Aether::Colour);

            // Returns string
            std::string string();

            // Custom highlight/select graphics
            void renderHighlighted();
            void renderSelected();
    };
};

#endif