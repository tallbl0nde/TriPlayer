#ifndef ELEMENT_IMAGE_HPP
#define ELEMENT_IMAGE_HPP

#include "Aether/Aether.hpp"

namespace CustomElm {
    // Class extending Aether's element providing a basic Image implementation
    // in order to allow instantiation from a drawable.
    class Image : public Aether::Element {
        private:
            // Colour to tint image with
            Aether::Colour colour_;

            // Drawable to render
            Aether::Drawable * drawable;

        public:
            // Constructs a new image element using the provided drawable.
            Image(int x, int y, Aether::Drawable * drawable);

            // Return the colour of the image.
            Aether::Colour colour();

            // Set the colour of the image.
            void setColour(const Aether::Colour &);

            // Render the image.
            void render();

            // Delete the drawable.
            ~Image();
    };
};

#endif
