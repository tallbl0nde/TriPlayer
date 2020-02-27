#include "Tick.hpp"

namespace Aether {
    Tick::Tick(int x, int y, unsigned int d) : Element(x, y, d, d) {
        this->circle = new Ellipse(this->x(), this->y(), d);
        this->addElement(this->circle);
        this->tick = new Text(0, 0, "\uE14B", 19, FontType::Extended);
        this->tick->setXY(this->x() + (this->w() - this->tick->w())/2 + 3, this->y() + (this->h() - this->tick->h())/2 + 3);
        this->addElement(this->tick);
    }

    unsigned int Tick::size() {
        return this->circle->xDiameter();
    }

    void Tick::setSize(unsigned int s) {
        this->setWH(s, s);
        this->circle->setXDiameter(s);
        this->circle->setYDiameter(s);
        this->tick->setXY(this->x() + (this->w() - this->tick->w()/2), this->y() + (this->h() - this->tick->h()/2));
    }

    Colour Tick::getCircleColour() {
        return this->circle->getColour();
    }

    void Tick::setCircleColour(Colour c) {
        this->circle->setColour(c);
    }

    Colour Tick::getTickColour() {
        return this->tick->getColour();
    }

    void Tick::setTickColour(Colour c) {
        this->tick->setColour(c);
    }

    void Tick::render() {
        SDL_BlendMode bld = SDLHelper::getBlendMode();
        SDLHelper::setBlendMode(SDL_BLENDMODE_BLEND);

        Element::render();

        SDLHelper::setBlendMode(bld);
    }
};