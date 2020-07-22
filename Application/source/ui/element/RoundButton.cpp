#include "ui/element/RoundButton.hpp"

namespace CustomElm {
    RoundButton::RoundButton(int x, int y, int d) : Aether::Element(x, y, d, d) {
        this->bg = Aether::Colour{255, 255, 255, 255};
        this->image = nullptr;
    }

    void RoundButton::positionImage() {
        this->image->setX(this->x() + (this->w() - this->image->w())/2);
        this->image->setY(this->y() + (this->h() - this->image->h())/2);
    }

    void RoundButton::setBackgroundColour(Aether::Colour c) {
        this->bg = c;
    }

    void RoundButton::setImage(Aether::Image * i) {
        this->removeElement(this->image);
        this->image = i;
        if (i != nullptr) {
            this->addElement(this->image);
            this->positionImage();
        }
    }

    void RoundButton::setW(int w) {
        Aether::Element::setW(w);
        this->positionImage();
    }

    void RoundButton::setH(int h) {
        Aether::Element::setH(h);
        this->positionImage();
    }

    SDL_Texture * RoundButton::renderHighlightBG() {
        return SDLHelper::renderFilledEllipse(this->w()/2, this->h()/2);
    }

    SDL_Texture * RoundButton::renderHighlight() {
        return SDLHelper::renderEllipse(this->w()/2, this->h()/2, this->hiSize);
    }

    SDL_Texture * RoundButton::renderSelection() {
        return SDLHelper::renderFilledEllipse(this->w()/2, this->h()/2);
    }
};