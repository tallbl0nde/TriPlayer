#include "Texture.hpp"

namespace Aether {
    Texture::Texture(int x, int y, SDL_Texture * t) : Element(x, y) {
        this->texture = nullptr;
        this->setTexture(t);
        this->colour = Colour{255, 255, 255, 255};
    }

    int Texture::texW() {
        return this->texW_;
    }

    int Texture::texH() {
        return this->texH_;
    }

    Colour Texture::getColour() {
        return this->colour;
    }

    void Texture::setColour(Colour c) {
        this->colour = c;
    }

    void Texture::setColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        this->setColour(Colour{r, g, b, a});
    }

    void Texture::getMask(int * dx, int * dy, int * dw, int * dh) {
        *dx = this->maskX;
        *dy = this->maskY;
        *dw = this->maskW;
        *dh = this->maskH;
    }

    void Texture::setMask(int dx, int dy, int dw, int dh) {
        this->maskX = dx;
        this->maskY = dy;
        this->maskW = dw;
        this->maskH = dh;
    }

    void Texture::setTexture(SDL_Texture * t) {
        this->destroyTexture();
        this->texture = t;
        SDLHelper::getDimensions(this->texture, &this->texW_, &this->texH_);
        this->setW(this->texW_);
        this->setH(this->texH_);
        this->setMask(0, 0, this->texW_, this->texH_);
    }

    void Texture::destroyTexture() {
        if (this->texture != nullptr) {
            SDLHelper::destroyTexture(this->texture);
        }
        this->texture = nullptr;
        this->texW_ = 0;
        this->texH_ = 0;
    }

    void Texture::render() {
        if (this->hidden()) {
            return;
        }

        SDLHelper::drawTexture(this->texture, this->colour, this->x(), this->y(), this->w(), this->h(), this->maskX, this->maskY, this->maskW, this->maskH);
        Element::render();
    }

    Texture::~Texture() {
        this->destroyTexture();
    }
};