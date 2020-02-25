#include "SideButton.hpp"

namespace CustomElm {
    SideButton::SideButton(int x, int y, int w) : Element(x, y, w, 60) {
        this->rect = new Aether::Rectangle(this->x() + 5, this->y() + 8, 7, 44, 2);
        this->addElement(this->rect);
        this->rect->setHidden(true);
        this->icon = nullptr;
        this->text = new Aether::Text(this->x() + 60, this->y() + this->h()/2, "", 26);
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
        this->icon->setWH(26, 26);
        this->icon->setXY(this->x() + 20, this->y() + (this->h() - this->icon->h())/2);
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
            this->rect->setHidden(false);
        } else {
            if (this->icon != nullptr) {
                this->icon->setColour(this->inactive);
            }
            this->text->setColour(this->inactive);
            this->rect->setHidden(true);
        }
    }
};