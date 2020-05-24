#include "MenuButton.hpp"

namespace CustomElm {
    MenuButton::MenuButton(int x, int y, int w, int h) : Element(x, y, w, h) {
        this->icon = nullptr;
        this->text = new Aether::Text(this->x() + 55, this->y() + this->h()/2, "", 24);
        this->addElement(this->text);
    }

    void MenuButton::setIconColour(Aether::Colour c) {
        if (this->icon != nullptr) {
            this->icon->setColour(c);
        }
    }

    void MenuButton::setTextColour(Aether::Colour c) {
        this->text->setColour(c);
    }

    void MenuButton::setIcon(Aether::Image * i) {
        if (this->icon != nullptr) {
            this->removeElement(this->icon);
        }
        this->icon = i;
        this->icon->setWH(26, 26);
        this->icon->setXY(this->x() + 15, this->y() + (this->h() - this->icon->h())/2);
        this->addElement(this->icon);
    }

    void MenuButton::setText(std::string s) {
        this->text->setString(s);
        if (this->text->w() > (this->x() + this->w()) - this->text->x()) {
            this->text->setW((this->x() + this->w()) - this->text->x());
        }
        this->text->setY(this->y() + (this->h() - this->text->h())/2);
    }
};