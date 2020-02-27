#ifndef AETHER_RECTANGLE_HPP
#define AETHER_RECTANGLE_HPP

#include "base/Texture.hpp"

namespace Aether {
    // A rectangle is a texture containing either a normal or rounded
    // rectangle. The object stores all relevant information so that
    // it can be regenerated whenever the dimensions/radius changes.
    class Rectangle : public Texture {
        private:
            // Radius of each corner (draws rounded rectangle when > 0)
            unsigned int cornerRadius_;

            // Called to regenerate texture
            void redrawTexture();

        public:
            // Constructor takes parent element and optional corner radius (defaults to 0)
            Rectangle(int, int, int, int, unsigned int = 0);

            // Getter + setter for cornerRadius
            unsigned int cornerRadius();
            void setCornerRadius(unsigned int);

            // Adjust rectangle size and render new texture
            void setRectSize(int, int);
    };
};

#endif