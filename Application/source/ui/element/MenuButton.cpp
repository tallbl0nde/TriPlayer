#include "ui/element/MenuButton.hpp"

// Height of a button
#define HEIGHT 60
// Size of image
#define IMAGE_SIZE 26

namespace CustomElm {
    MenuButton::MenuButton() : Element(0, 0, 100, HEIGHT) {
        this->icon = nullptr;
        this->text = new Aether::Text(this->x() + 55, this->y() + this->h()/2, "", 24);
        this->text->setScroll(false);
        this->text->setScrollSpeed(60);
        this->text->setScrollWaitTime(1000);
        this->addElement(this->text);
    }

    void MenuButton::positionItems() {
        int maxW = (this->x() + this->w()) - this->text->x() - HEIGHT/4;
        if (this->text->texW() > maxW) {
            this->text->setW(maxW);
        } else {
            this->text->setW(this->text->texW());
        }
    }

    void MenuButton::update(uint32_t dt) {
        Element::update(dt);

        if (this->highlighted() && !this->text->scroll()) {
            this->text->setScroll(true);
        } else if (!this->highlighted() && this->text->scroll()) {
            this->text->setScroll(false);
        }
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
        this->removeElement(this->icon);
        this->icon = i;
        this->icon->setWH(IMAGE_SIZE, IMAGE_SIZE);
        this->icon->setXY(this->x() + HEIGHT/4, this->y() + (this->h() - this->icon->h())/2);
        this->addElement(this->icon);
    }

    void MenuButton::setText(std::string s) {
        this->text->setString(s);
        this->text->setY(this->y() + (this->h() - this->text->h())/2);
        this->positionItems();
    }

    void MenuButton::setW(int w) {
        Element::setW(w);
        this->positionItems();
    }
};