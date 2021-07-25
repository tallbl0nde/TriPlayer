#ifndef ELEMENT_BUTTONPICKER_HPP
#define ELEMENT_BUTTONPICKER_HPP

#include "Aether/Aether.hpp"

// Special element that listens for a button press
namespace CustomElm {
    class ButtonPicker : public Aether::Element {
        private:
            Aether::Button button;        // Selected button
            Aether::Text * str;             // Button string
            Aether::Box * rect;             // Bounding rectangle

            Aether::Colour inactive;
            Aether::Colour active;

            // Set new text string and recenter
            void updateText();

        public:
            // Constructor takes position (size is hardcoded) and initial button
            ButtonPicker(int, int, Aether::Button);

            // Override to intercept button press and update containing button
            bool handleEvent(Aether::InputEvent *);
            void setSelected(bool);
            Aether::Drawable * renderHighlight();
            Aether::Drawable * renderSelection();

            // Get the currently 'chosen' button
            Aether::Button selectedButton();
            // Set currently 'chosen' button
            void setSelectedButton(const Aether::Button);

            // Set colours
            void setRectangleColour(const Aether::Colour &);
            void setActiveColour(const Aether::Colour &);
            void setInactiveColour(const Aether::Colour &);
    };
};

#endif
