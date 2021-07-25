#ifndef ELEMENT_TEXTBOX_HPP
#define ELEMENT_TEXTBOX_HPP

#include "Aether/Aether.hpp"
#include "utils/NX.hpp"

namespace CustomElm {
    // A text box opens up the on screen keyboard when
    class TextBox : public Aether::Element {
        private:
            // Round rectangle 'background'
            Aether::Rectangle * rect;
            // Text string
            Aether::Text * text;
            // Callback function
            std::function<void()> textFunc;

            // Keyboard config
            Utils::NX::Keyboard conf;

            // Opens the keyboard
            void openKeyboard();
            // Function called to position text after edit
            void positionText();

        public:
            // x, y, w, h (font size calculated based on height)
            TextBox(int, int, int, int);

            // Set colours
            void setBoxColour(Aether::Colour);
            void setTextColour(Aether::Colour);

            // Set callback (when string changed)
            void setKeyboardCallback(std::function<void()>);
            // Set keyboard hint
            void setKeyboardHint(std::string);
            // Set max number of characters permitted
            void setKeyboardLimit(size_t);

            // Set stored string
            void setString(std::string);
            // Returns stored string
            std::string string();

            // Custom highlight/select graphics
            Aether::Drawable * renderHighlightBG();
            Aether::Drawable * renderHighlight();
            Aether::Drawable * renderSelection();
    };
};

#endif
