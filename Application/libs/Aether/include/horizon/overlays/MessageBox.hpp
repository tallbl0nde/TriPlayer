#ifndef AETHER_MESSAGEBOX_HPP
#define AETHER_MESSAGEBOX_HPP

#include "horizon/button/BorderButton.hpp"
#include "primary/Rectangle.hpp"
#include "Overlay.hpp"

namespace Aether {
    // A "MessageBox" is a box presented in the middle of the screen,
    // usually containing a few yes/no/cancel buttons and some text.
    // Buttons are set up using the provided functions whereas the
    // "body" of the box is set by giving an element containing the
    // desired elements.
    class MessageBox : public Overlay {
        private:
            // Buttons
            BorderButton * left;        // Bottom left
            BorderButton * right;       // Bottom right
            BorderButton * top;         // Top

            // Lines/separators
            Rectangle * topH;
            Rectangle * bottomH;
            Rectangle * vertH;

            // Element being used as body (set to nullptr if nothing)
            // Anchored at top left of rectangle
            Element * body;

            // Rectangle "background"
            Rectangle * rect;

            void repositionButtons();
            void resizeElements();

        public:
            // Constructor takes nothing but sets up rect
            MessageBox();

            // Set colours (must be called after adding buttons)
            void setLineColour(Colour);
            void setRectangleColour(Colour);
            void setTextColour(Colour);

            // Add buttons - layout is done automatically
            // Don't call these multiple times as the previous ones aren't deleted!
            void addLeftButton(std::string, std::function<void()>);
            void addRightButton(std::string, std::function<void()>);
            void addTopButton(std::string, std::function<void()>);

            // Returns available width and height for body in given pointers
            void getBodySize(int *, int *);
            // Set the size of the body (not overall container)
            void setBodySize(int, int);

            // Deletes the element (and it's children!) used as body
            void emptyBody();
            // Accepts element to use as body (should contain other elements)
            // Does nothing if a body has been set!
            void setBody(Element *);
    };
};

#endif