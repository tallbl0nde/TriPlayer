#ifndef AETHER_SCROLLABLE_HPP
#define AETHER_SCROLLABLE_HPP

#include "Container.hpp"

namespace Aether {
    // A scrollable element has it's children rendered onto a
    // texture, which is then "scrolled" and only a portion of
    // it rendered (useful for lists)
    class Scrollable : public Container {
        private:
            // Texture to render to
            SDL_Texture * renderTex;
            // Scroll bar texture
            static SDL_Texture * scrollBar;
            // Colour to tint scroll bar
            Colour scrollBarColour;
            // Show the scrollbar?
            bool showScrollBar_;

            // Check all children and determine maximum height
            void updateMaxScrollPos();

            // Stops scrolling and sets active element
            void stopScrolling();

        protected:
            // Is scrolling allowed? (set by function)
            bool canScroll_;
            // Has this scrollable been touched? (used for scrolling when touch is outside)
            bool isTouched;
            // Is the element scrolling? (used for touch events)
            bool isScrolling;
            // Amount to decrease velocity by (per second)
            float scrollDampening;
            // Scroll velocity (amount to scroll per second)
            float scrollVelocity;
            // Start of touch Y coord (used to touch instead of scroll briefly)
            int touchY;

            // Maximum Y offset in pixels
            unsigned int maxScrollPos;
            // Amount to "catchup" by
            float scrollCatchup;
            // Offset (y) in pixels
            unsigned int scrollPos;

            // Set scrollPos but check if going to be outside of range
            // and if so set to min/max
            void setScrollPos(int);

        public:
            // X, Y, W, H of scrollable object
            Scrollable(int, int, int, int);

            // Setting width needs to adjust width of elements
            void setW(int);
            // Setting height needs to recalculate scroll bar
            void setH(int);

            // Getter + setter for catchup
            int catchup();
            void setCatchup(int);

            // Getter + Setter for dampening
            float dampening();
            void setDampening(float);

            // Getter + Setter for showing scroll bar
            bool showScrollBar();
            void setShowScrollBar(bool);
            // Set scroll bar colour
            void setScrollBarColour(Colour);
            void setScrollBarColour(uint8_t, uint8_t, uint8_t, uint8_t);

            // Get/set whether this element can scroll (by touch/drag)
            bool canScroll();
            void setCanScroll(bool);

            // (Re-)calculate maximum scroll position whenever an element is added/removed
            void addElement(Element *);
            bool removeElement(Element *);
            void removeAllElements();
            // Advanced: removes ny elements added _AFTER_ the given element, disregarding position in list!!
            bool removeFollowingElements(Element *);

            // Handles scrolling
            bool handleEvent(InputEvent *);
            // Update handles scrolling after touch up
            void update(uint32_t);
            // Render also draws scroll bar if applicable
            void render();

            // Delete scroll bar texture
            ~Scrollable();
    };
};

#endif