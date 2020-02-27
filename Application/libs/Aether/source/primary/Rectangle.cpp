#include "Rectangle.hpp"

namespace Aether {
    Rectangle::Rectangle(int x, int y, int w, int h, unsigned int r) : Texture(x, y) {
        Texture::setW(w);
        Texture::setH(h);
        this->setCornerRadius(r);
    }

    void Rectangle::redrawTexture() {
        if (this->cornerRadius_ > 0) {
            this->setTexture(SDLHelper::renderFilledRoundRect(this->w(), this->h(), this->cornerRadius_));
        } else {
            this->setTexture(SDLHelper::renderFilledRect(this->w(), this->h()));
        }
    }

    unsigned int Rectangle::cornerRadius() {
        return this->cornerRadius_;
    }

    void Rectangle::setCornerRadius(unsigned int r) {
        this->cornerRadius_ = r;
        this->redrawTexture();
    }

    void Rectangle::setRectSize(int w, int h) {
        Texture::setW(w);
        Texture::setH(h);
        this->redrawTexture();
    }
};