#ifndef ELEMENT_ROUNDBUTTON_HPP
#define ELEMENT_ROUNDBUTTON_HPP

#include "Aether/Aether.hpp"

// TODO: Cache circle textures and delete when unselected

namespace CustomElm {
    // A RoundButton is a 'wrapper' for an image to show a circular
    // selection, etc
    class RoundButton : public Aether::Element {
        private:
            // Background colour
            Aether::Colour bg;
            // Pointer to contained image
            Aether::Image * image;

            // Center image
            void positionImage();

        public:
            RoundButton(int, int, int);

            void setBackgroundColour(Aether::Colour);
            void setImage(Aether::Image *);

            // Need to recenter Image on dimension change
            void setW(int);
            void setH(int);

            // Override rendering selection and highlight to make it round
            SDL_Texture * renderHighlightBG();
            SDL_Texture * renderHighlight();
            SDL_Texture * renderSelection();
    };
};

#endif