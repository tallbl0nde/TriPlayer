#include "BorderButton.hpp"

// Corner radius of rectangle
#define CORNER_RAD 8

namespace Aether {
    BorderButton::BorderButton(int x, int y, int w, int h, unsigned int b, std::string t, unsigned int s, std::function<void()> f) : Element(x, y, w, h) {
        this->box = new Box(this->x(), this->y(), this->w(), this->h(), b, CORNER_RAD);
        this->text = new Text(this->x() + this->w()/2, this->y() + this->h()/2, t, s);
        this->text->setXY(this->text->x() - this->text->w()/2, this->text->y() - this->text->h()/2);
        this->text->setScroll(true);
        this->addElement(this->box);
        this->addElement(this->text);
        this->setCallback(f);
    }

    Colour BorderButton::getBorderColour() {
        return this->box->getColour();
    }

    void BorderButton::setBorderColour(Colour c) {
        this->box->setColour(c);
    }

    Colour BorderButton::getTextColour() {
        return this->text->getColour();
    }

    void BorderButton::setTextColour(Colour c) {
        this->text->setColour(c);
    }

    std::string BorderButton::getString() {
        return this->text->string();
    }

    void BorderButton::setString(std::string s) {
        this->text->setString(s);
        if (this->text->texW() > this->w()) {
            this->text->setW(this->w());
        }
        this->text->setX(this->x() + this->w()/2 - this->text->w()/2);
    }

    void BorderButton::setW(int w) {
        Element::setW(w);
        this->box->setBoxSize(this->w(), this->h());
        this->text->setX(this->x() + this->w()/2 - this->text->w()/2);
    }

    void BorderButton::setH(int h) {
        Element::setH(h);
        this->box->setBoxSize(this->w(), this->h());
        this->text->setY(this->y() + this->h()/2 - this->text->h()/2);
    }

    void BorderButton::render() {
        Element::render();

        // Now render highlight over the top
        if (this->highlighted() && !this->isTouch) {
            SDLHelper::drawRoundRect(this->hiBorder, this->x() - this->hiSize + this->box->border(), this->y() - this->hiSize + this->box->border(), this->w() + 2*(this->hiSize - this->box->border()), this->h() + 2*(this->hiSize - this->box->border()), CORNER_RAD + 2, this->hiSize);
        }
    }

    void BorderButton::renderHighlighted() {
        // Draw background
        SDLHelper::drawFilledRoundRect(this->hiBG, this->x(), this->y(), this->w(), this->h(), CORNER_RAD + 2);
    }

    void BorderButton::renderSelected() {
        SDLHelper::drawFilledRoundRect(this->hiSel, this->x(), this->y(), this->w(), this->h(), CORNER_RAD + 2);
    }
};