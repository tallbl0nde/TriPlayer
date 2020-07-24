#ifndef OVERLAY_MENU_HPP
#define OVERLAY_MENU_HPP

#include "ui/element/MenuButton.hpp"

namespace CustomOvl {
    // A menu is the overlay that appears when 'more' is pressed on an element.
    // This is the base class which doesn't have the image section, only buttons
    class Menu : public Aether::Overlay {
        private:
            // Container storing buttons for easy alignment
            Aether::Container * btns;
            // Top element (used to refocus)
            CustomElm::MenuButton * topBtn;
            // Was touch pressed outside of rectangle?
            bool touchOutside;

        protected:
            // Rectangle 'background'
            Aether::Rectangle * bg;
            // Position to insert next element
            int nextY;

        public:
            // Constructor initializes a few things
            Menu();

            // Adds a button to the list
            // Pass nullptr to finalize list
            void addButton(CustomElm::MenuButton *);
            // Add a separator at the current end of the list with the given colour
            void addSeparator(Aether::Colour);

            // Set colours
            void setBackgroundColour(Aether::Colour);

            // Close menu if tapped outside of rectangle
            bool handleEvent(Aether::InputEvent *);

            // Move the highlight back to the top element
            void resetHighlight();
    };
};

#endif