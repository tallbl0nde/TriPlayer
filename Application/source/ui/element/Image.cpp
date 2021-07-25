#include "ui/element/Image.hpp"

namespace CustomElm {
    Image::Image(int x, int y, Aether::Drawable * drawable) : Aether::Element(x, y, drawable->width(), drawable->height()) {
        this->colour_ = Aether::Colour(255, 255, 255, 255);
        this->drawable = drawable;
        this->drawable->convertToTexture();
        this->drawable->setColour(this->colour_);
    }

    Aether::Colour Image::colour() {
        return this->colour_;
    }

    void Image::setColour(const Aether::Colour & col) {
        this->colour_ = col;
        this->drawable->setColour(col);
    }

    void Image::render() {
        if (this->hidden()) {
            return;
        }

        this->drawable->render(this->x(), this->y(), this->w(), this->h());
        Element::render();
    }

    Image::~Image() {
        delete this->drawable;
    }
};
