#ifndef ELEMENT_NUMBERBOX_HPP
#define ELEMENT_NUMBERBOX_HPP

#include "Aether/Aether.hpp"
#include "utils/NX.hpp"

namespace CustomElm {
    // A number box opens the on screen numpad and prompts for an integer
    // to be entered.
    class NumberBox : public Aether::Element {
        private:
            // Round rectangle 'background'
            Aether::Rectangle * rect;
            // Text string
            Aether::Text * text;
            // Callback function
            std::function<void()> textFunc;

            // Numpad config
            Utils::NX::Numpad conf;

            // Opens the numpad
            void openNumpad();
            // Function called to position text after edit
            void positionText();

        public:
            // x, y, w, h (font size calculated based on height)
            NumberBox(int, int, int, int);

            // Set colours
            void setBoxColour(Aether::Colour);
            void setTextColour(Aether::Colour);

            // Set whether to allow floating point numbers
            void setNumpadAllowFloats(bool);
            // Set callback (when string changed)
            void setNumpadCallback(std::function<void()>);
            // Set keyboard hint
            void setNumpadHint(std::string);
            // Set max number of digits permitted
            void setNumpadLimit(size_t);
            // Set whether to allow negative digits
            void setNumpadNegative(bool);

            // Set stored value
            void setValue(int);
            // Returns stored value
            int value();

            // Custom highlight/select graphics
            SDL_Texture * renderHighlightBG();
            SDL_Texture * renderHighlight();
            SDL_Texture * renderSelection();
    };
};

#endif