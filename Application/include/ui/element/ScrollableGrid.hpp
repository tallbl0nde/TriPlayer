#ifndef ELEMENT_SCROLLABLEGRID_HPP
#define ELEMENT_SCROLLABLEGRID_HPP

#include "Aether/Aether.hpp"

namespace CustomElm {
    // This is a copy of Aether's Scrollable/List classes but instead of expecting rows
    // it expects and handles everything in a grid
    class ScrollableGrid : public Aether::Container {
        private:
            // Grid dimensions
            unsigned int rowHeight;
            unsigned int cols;

            // Scrollable vars
            bool isScrolling;
            bool isTouched;
            int scrollVelocity;
            int scrollPos;
            int maxScrollPos;
            SDL_Texture * scrollBar;
            Aether::Colour scrollBarColour;
            bool showScrollBar_;
            int touchY;

            // Positions given child (x and y) based on index
            void positionChild(Aether::Element *, size_t);

            // Scrollable methods
            void setScrollPos(int);
            void stopScrolling();
            void updateMaxScrollPos();

        public:
            // Constructor accepts:
            // x, y, w, h,
            // row height, no. of columns
            ScrollableGrid(int, int, int, int, unsigned int, unsigned int);

            // Adding an element adds it to the next available column in a row
            // Doesn't resize as a scrollable normally does - just positions!
            void addElement(Aether::Element *);
            void removeAllElements();

            // Implements required behaviour
            bool handleEvent(Aether::InputEvent *);
            void update(uint32_t);
            void render();

            // Need to reposition children
            void setW(int);
            void setH(int);

            // Scrollable methods
            void setShowScrollBar(bool);
            void setScrollBarColour(Aether::Colour);

            ~ScrollableGrid();
    };
};

#endif