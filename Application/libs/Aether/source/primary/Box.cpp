#include "Box.hpp"

namespace Aether {
    Box::Box(int x, int y, int w, int h, unsigned int b, unsigned int r) : Texture(x, y) {
        Texture::setW(w);
        Texture::setH(h);
        this->cornerRadius_ = r;
        this->setBorder(b);
    }

    void Box::redrawTexture() {
        if (this->cornerRadius_ > 0) {
            this->setTexture(SDLHelper::renderRoundRect(this->w(), this->h(), this->cornerRadius_, this->border()));
        } else {
            this->setTexture(SDLHelper::renderRect(this->w(), this->h(), this->border()));
        }
    }

    unsigned int Box::border() {
        return this->border_;
    }

    void Box::setBorder(unsigned int b) {
        this->border_ = b;
        this->redrawTexture();
    }

    void Box::setBoxSize(int w, int h) {
        Texture::setW(w);
        Texture::setH(h);
        this->redrawTexture();
    }

    unsigned int Box::cornerRadius() {
        return this->cornerRadius_;
    }

    void Box::setCornerRadius(unsigned int r) {
        this->cornerRadius_ = r;
        this->redrawTexture();
    }
};
