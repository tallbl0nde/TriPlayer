#ifndef ELEMENT_HORIZONTALLIST_HPP
#define ELEMENT_HORIZONTALLIST_HPP

#include "Aether/Aether.hpp"

namespace CustomElm {
    // Essentially the same as a typical vertical list but scrolls horizontally
    // For a complete explanation of everything see Aether::Scrollable
    // Note that instead of stretching the width, the height of children will be set!
    class HorizontalList : public Aether::Container {
        private:
            // Scrollable variables
            bool isScrolling;
            bool isTouched;
            int maxScrollPos;
            int scrollVelocity;
            int scrollPos;
            int touchX;
            int touchY;

            // Scrollable methods
            void setScrollPos(int);
            void stopScrolling();
            void updateMaxScrollPos();

        public:
            // Constructor accepts position and dimensions
            HorizontalList(int, int, int, int);

            // Implements similar behaviour to scrollable
            void addElement(Aether::Element *);
            void removeAllElements();

            bool handleEvent(Aether::InputEvent *);
            void update(uint32_t);
            void render();

            // Need to reposition children on dimension change
            void setW(int);
            void setH(int);
    };
};

#endif