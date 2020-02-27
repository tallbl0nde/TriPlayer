#ifndef AETHER_TEXTURE_HPP
#define AETHER_TEXTURE_HPP

#include "Element.hpp"

namespace Aether {
    // A Texture is an element consisting of an SDL_Texture.
    // It stores the width and height of the texture and the
    // colour it's tinted with.
    class Texture : public Element {
        private:
            // Dimensions of the texture
            int texW_, texH_;

            // Coords + dimensions of OVERALL texture to draw
            int maskX, maskY, maskW, maskH;

            // Delete the stored texture (only called internally)
            void destroyTexture();

        protected:
            // Colour to tint the texture with when drawn
            // Defaults to white
            Colour colour;

            // Overriden by derived classes to redraw texture when
            // variables are changed
            virtual void redrawTexture() = 0;

            // The actual texture
            SDL_Texture * texture;

        public:
            // Constructor optionally takes the texture + sets dimensions to those of the texture
            Texture(int, int, SDL_Texture * = nullptr);

            // Getters for dimensions
            int texW();
            int texH();

            // Return the set colour
            Colour getColour();
            // Set the colour
            void setColour(Colour);
            void setColour(uint8_t, uint8_t, uint8_t, uint8_t);

            // Set pointed values to values of mask
            void getMask(int *, int *, int *, int *);
            // Set the "mask" of the texture to draw
            void setMask(int, int, int, int);

            // Set the texture and recalculate dimensions (also destroys previous one)
            void setTexture(SDL_Texture *);

            // Render the texture
            void render();

            // Destructor destroys stored texture
            ~Texture();
    };
};

#endif