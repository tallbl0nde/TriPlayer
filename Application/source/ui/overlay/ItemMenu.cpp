#include "ui/overlay/ItemMenu.hpp"

// Size of image
#define IMAGE_SIZE 85
// Padding around image
#define PADDING 10

namespace CustomOvl {
    ItemMenu::ItemMenu() : Menu() {
        this->top = new Aether::Element(this->bg->x() + 20, this->bg->y() + this->nextY, this->bg->w() - 40, IMAGE_SIZE + 2*PADDING);
        this->image = nullptr;
        this->mainText = new Aether::Text(this->top->x() + IMAGE_SIZE + 2*PADDING, this->top->y() + 30, "", 24);
        this->mainText->setScroll(true);
        this->mainText->setScrollSpeed(35);
        this->mainText->setScrollWaitTime(1200);
        this->top->addElement(this->mainText);
        this->subText = new Aether::Text(this->top->x() + IMAGE_SIZE + 2*PADDING, this->mainText->y() + 32, "", 18);
        this->top->addElement(this->subText);
        this->bg->addElement(this->top);

        // Set insert position to bottom of this section
        this->nextY += this->top->h() + 5;
    }

    void ItemMenu::setImage(Aether::Image * i) {
        this->top->removeElement(this->image);
        this->image = i;
        this->image->setXY(this->top->x() + 5, this->top->y() + 15);
        this->image->setWH(IMAGE_SIZE, IMAGE_SIZE);
        this->top->addElement(this->image);
    }

    void ItemMenu::setMainText(const std::string & s) {
        this->mainText->setString(s);
        int maxW = this->top->x() + this->top->w() - this->mainText->x() - PADDING;
        if (this->mainText->w() > maxW) {
            this->mainText->setW(maxW);
        }
    }

    void ItemMenu::setSubText(const std::string & s) {
        this->subText->setString(s);
        int maxW = this->top->x() + this->top->w() - this->subText->x() - PADDING;
        if (this->subText->w() > maxW) {
            this->subText->setW(maxW);
        }
    }

    void ItemMenu::setMainTextColour(Aether::Colour c) {
        this->mainText->setColour(c);
    }

    void ItemMenu::setSubTextColour(Aether::Colour c) {
        this->subText->setColour(c);
    }
}