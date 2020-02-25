#include "SideButton.hpp"

namespace CustomElm {
    SideButton::SideButton(int x, int y, int w) : Element(x, y, w, 50) {
        this->rect = new Aether::Rectangle(this->x() + 5, this->y() + 5, 5, 40, 2);
        this->addElement(this->rect);
        this->rect->setHidden(true);
        this->icon = nullptr;
        this->text = new Aether::Text(this->x() + 60, this->y() + this->h()/2, "", 24);
        this->addElement(this->text);
        this->isActive = false;
    }

    void SideButton::setInactiveColour(Aether::Colour c) {
        this->inactive = c;
        if (!this->isActive) {
            if (this->icon != nullptr) {
                this->icon->setColour(c);
            }
            this->text->setColour(c);
        }
    }

    void SideButton::setActiveColour(Aether::Colour c) {
        this->active = c;
        if (this->isActive) {
            if (this->icon != nullptr) {
                this->icon->setColour(c);
            }
            this->text->setColour(c);
        }
        this->rect->setColour(c);
    }

    void SideButton::setIcon(Aether::Image * i) {
        if (this->icon != nullptr) {
            this->removeElement(this->icon);
        }
        this->icon = i;
        this->icon->setXY(this->x() + 10, this->y() + 15);
        this->icon->setWH(20, 20);
        this->addElement(this->icon);
    }

    void SideButton::setText(std::string s) {
        this->text->setString(s);
        if (this->text->w() > (this->x() + this->w()) - this->text->x()) {
            this->text->setW((this->x() + this->w()) - this->text->x());
        }
        this->text->setY(this->y() + (this->h() - this->text->h())/2);
    }

    void SideButton::setActivated(bool b) {
        this->isActive = b;
        if (this->isActive) {
            if (this->icon != nullptr) {
                this->icon->setColour(this->active);
            }
            this->text->setColour(this->active);
            this->rect->setHidden(true);
        } else {
            if (this->icon != nullptr) {
                this->icon->setColour(this->inactive);
            }
            this->text->setColour(this->inactive);
            this->rect->setHidden(false);
        }
    }
};