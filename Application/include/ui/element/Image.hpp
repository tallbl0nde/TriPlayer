#ifndef ELEMENT_IMAGE_HPP
#define ELEMENT_IMAGE_HPP

#include "Aether/Aether.hpp"

// Extends Aether::Texture to allow Image creation from an SDL_Surface
// It thus is created with RenderType::OnCreate
namespace CustomElm {
    class Image : public Aether::Texture {
        private:
            // Does nothing but is required to be instantiated
            void generateSurface();

        public:
            // Constructor takes x, y, and surface pointer
            Image(int, int, SDL_Surface *);
    };
};

#endif