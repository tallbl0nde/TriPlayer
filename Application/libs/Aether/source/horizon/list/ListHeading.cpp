#include "ListHeading.hpp"

// Text font size
#define FONT_SIZE 20
// Gap between rectangle and text
#define GAP 13
// Rectangle dimensions
#define RECT_W 5
#define RECT_H 30

namespace Aether {
    ListHeading::ListHeading(std::string s) : Element() {
        this->setH(RECT_H);
        this->rect = new Aether::Rectangle(this->x(), this->y(), RECT_W, RECT_H);
        this->addElement(rect);
        this->text = new Aether::Text(this->x() + RECT_W + GAP, this->y() + this->h()/2, s, FONT_SIZE);
        this->text->setY(this->text->y() - this->text->texH()/2);
        this->addElement(text);
        this->setW(this->rect->w() + GAP + this->text->w());
    }

    Colour ListHeading::getRectColour() {
        return this->rect->getColour();
    }

    void ListHeading::setRectColour(Colour c) {
        this->rect->setColour(c);
    }

    Colour ListHeading::getTextColour() {
        return this->text->getColour();
    }

    void ListHeading::setTextColour(Colour c) {
        this->text->setColour(c);
    }
}