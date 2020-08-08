#include "ui/element/Image.hpp"

namespace CustomElm {
    Image::Image(int x, int y, SDL_Surface * s) : Aether::Texture(x, y, Aether::RenderType::OnCreate){
        // Convert the surface immediately
        this->surface = s;
        this->convertSurface();
    }

    void Image::generateSurface() {

    }
};