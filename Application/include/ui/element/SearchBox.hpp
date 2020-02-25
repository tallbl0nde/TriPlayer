#ifndef ELEMENT_SEARCHBOX_HPP
#define ELEMENT_SEARCHBOX_HPP

#include "Aether.hpp"

namespace CustomElm {
    class SearchBox : public Aether::Element {
        private:
            // Round rect
            Aether::Box * rect;

            // Search icon
            Aether::Image * icon;

            // Search text
            Aether::Text * text;

        public:
            // x, y, w, h
            SearchBox(int, int, int, int);

            // Set colour of everything
            void setColour(Aether::Colour);

            // Returns string
            std::string string();

            // Custom highlight/select graphics
            void render();
            void renderHighlighted();
            void renderSelected();
    };
};

#endif