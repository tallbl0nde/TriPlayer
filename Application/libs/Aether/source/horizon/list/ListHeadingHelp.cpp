#include "ListHeadingHelp.hpp"

// Size of help icon (and thus height)
#define ICON_SIZE 30

namespace Aether {
    ListHeadingHelp::ListHeadingHelp(std::string s, std::function<void()> f) : Container() {
        Container::setH(ICON_SIZE);
        this->heading = new Aether::ListHeading(s);
        this->addElement(this->heading);
        this->help = new Aether::HelpButton(0, 0, ICON_SIZE, f);
        this->addElement(this->help);
        this->positionElements();
    }

    void ListHeadingHelp::positionElements() {
        this->heading->setXY(this->x(), this->y() + this->h()/2 - this->heading->h()/2);
        this->help->setXY(this->x() + this->w() - this->help->w(), this->y() + this->h()/2 - this->help->h()/2);
    }

    Colour ListHeadingHelp::getHelpColour() {
        return this->help->getColour();
    }

    void ListHeadingHelp::setHelpColour(Colour c) {
        this->help->setColour(c);
    }

    Colour ListHeadingHelp::getRectColour() {
        return this->heading->getRectColour();
    }

    void ListHeadingHelp::setRectColour(Colour c) {
        this->heading->setRectColour(c);
    }

    Colour ListHeadingHelp::getTextColour() {
        return this->heading->getTextColour();
    }

    void ListHeadingHelp::setTextColour(Colour c) {
        this->heading->setTextColour(c);
    }

    void ListHeadingHelp::setW(int w) {
        Container::setW(w);
        this->positionElements();
    }

    void ListHeadingHelp::setH(int h) {
        Container::setH(h);
        this->positionElements();
    }
};