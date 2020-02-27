#include "Ellipse.hpp"

namespace Aether {
    Ellipse::Ellipse(int x, int y, unsigned int xd, unsigned int yd) : Texture(x, y) {
        this->xDiameter_ = xd;
        this->yDiameter_ = ((yd == 0) ? xd : yd);
        this->redrawTexture();
    }

    void Ellipse::redrawTexture() {
        this->setTexture(SDLHelper::renderEllipse(this->xDiameter_, this->yDiameter_));
    }

    unsigned int Ellipse::xDiameter() {
        return this->xDiameter_;
    }

    void Ellipse::setXDiameter(unsigned int xd) {
        this->xDiameter_ = xd;
    }

    unsigned int Ellipse::yDiameter() {
        return this->yDiameter_;
    }

    void Ellipse::setYDiameter(unsigned int yd) {
        this->yDiameter_ = yd;
    }

    void Ellipse::renderHighlighted() {
        int midX = this->x() + this->w()/2;
        int midY = this->y() + this->h()/2;

        // Draw background
        SDLHelper::drawEllipse(this->hiBG, midX, midY, this->w(), this->h());

        // Draw outline
        SDLHelper::drawEllipse(this->hiBorder, midX, midY, this->w() + 2*this->hiSize, this->h() + 2*this->hiSize);
    }

    void Ellipse::renderSelected() {
        SDLHelper::drawEllipse(this->hiSel, this->x() + this->w()/2, this->y() + this->h()/2, this->w(), this->h());
    }
};