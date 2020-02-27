#ifndef AETHER_IMAGE_HPP
#define AETHER_IMAGE_HPP

#include "base/Texture.hpp"

namespace Aether {
    // An image is literally a texture that is created from an image.
    // Not much more to it than that!
    class Image : public Texture {
        private:
            // Does nothing
            void redrawTexture();

        public:
            // Both constructors take coordinates
            // Takes path to image to read and render
            Image(int, int, std::string);
            // Takes pointer to image and size + optional factors to scale down (advanced!)
            Image(int, int, u8 *, size_t, int = 1, int = 1);
    };
};

#endif