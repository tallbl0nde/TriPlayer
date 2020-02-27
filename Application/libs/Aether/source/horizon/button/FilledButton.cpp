#include "FilledButton.hpp"

// Corner radius of rectangle
#define CORNER_RAD 8

namespace Aether {
    FilledButton::FilledButton(int x, int y, int w, int h, std::string t, unsigned int s, std::function<void()> f) : Element(x, y, w, h) {
        this->rect = new Rectangle(this->x(), this->y(), this->w(), this->h(), CORNER_RAD);
        this->text = new Text(this->x() + this->w()/2, this->y() + this->h()/2, t, s);
        this->text->setXY(this->text->x() - this->text->w()/2, this->text->y() - this->text->h()/2);
        this->text->setScroll(true);
        this->addElement(this->rect);
        this->addElement(this->text);
        this->setCallback(f);
    }

    Colour FilledButton::getFillColour() {
        return this->rect->getColour();
    }

    void FilledButton::setFillColour(Colour c) {
        this->rect->setColour(c);
    }

    Colour FilledButton::getTextColour() {
        return this->text->getColour();
    }

    void FilledButton::setTextColour(Colour c) {
        this->text->setColour(c);
    }

    std::string FilledButton::getString() {
        return this->text->string();
    }

    void FilledButton::setString(std::string s) {
        this->text->setString(s);
        if (this->text->texW() > this->w()) {
            this->text->setW(this->w());
        }
        this->text->setX(this->x() + this->w()/2 - this->text->w()/2);
    }

    void FilledButton::setW(int w) {
        Element::setW(w);
        this->rect->setRectSize(this->w(), this->h());
        this->text->setX(this->x() + this->w()/2 - this->text->w()/2);
    }

    void FilledButton::setH(int h) {
        Element::setH(h);
        this->rect->setRectSize(this->w(), this->h());
        this->text->setY(this->y() + this->h()/2 - this->text->h()/2);
    }

    void FilledButton::renderHighlighted() {
        // Draw background
        SDLHelper::drawFilledRoundRect(this->hiBG, this->x(), this->y(), this->w(), this->h(), CORNER_RAD + 2);

        // Draw outline
        SDLHelper::drawRoundRect(this->hiBorder, this->x() - this->hiSize - 2, this->y() - this->hiSize - 2, this->w() + 2*this->hiSize + 2, this->h() + 2*this->hiSize + 2, CORNER_RAD + 2, this->hiSize);
    }

    void FilledButton::renderSelected() {

    }
};