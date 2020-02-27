#include "ListButton.hpp"

// Text size
#define FONT_SIZE 22
// Height of item
#define HEIGHT 70
// Pixels to indent text
#define TEXT_PADDING 16

namespace Aether {
    ListButton::ListButton(std::string s, std::function<void()> f) : Element() {
        Element::setH(HEIGHT);

        this->topR = new Rectangle(this->x(), this->y(), this->w(), 1);
        this->addElement(this->topR);
        this->bottomR = new Rectangle(this->x(), this->y() + this->h(), this->w(), 1);
        this->addElement(this->bottomR);

        this->text_ = new Text(this->x() + TEXT_PADDING, this->y() + this->h()/2, s, FONT_SIZE);
        this->text_->setY(this->text_->y() - this->text_->h()/2);
        this->addElement(this->text_);

        this->setCallback(f);
    }

    Colour ListButton::getLineColour() {
        return this->topR->getColour();
    }

    void ListButton::setLineColour(Colour c) {
        this->topR->setColour(c);
        this->bottomR->setColour(c);
    }

    Colour ListButton::getTextColour() {
        return this->text_->getColour();
    }

    void ListButton::setTextColour(Colour c) {
        this->text_->setColour(c);
    }

    std::string ListButton::text() {
        return this->text_->string();
    }

    void ListButton::setText(std::string s) {
        this->text_->setString(s);
    }

    unsigned int ListButton::fontSize() {
        return this->text_->fontSize();
    }

    void ListButton::setFontSize(unsigned int f) {
        this->text_->setFontSize(f);
        this->text_->setY(this->y() + (this->h() - this->text_->h())/2);
    }

    void ListButton::setW(int w) {
        Element::setW(w);
        this->topR->setRectSize(this->w(), 1);
        this->bottomR->setRectSize(this->w(), 1);
    }

    void ListButton::setH(int h) {
        Element::setH(h);
        this->bottomR->setY(this->h());
        this->text_->setY(this->y() + (this->h() - this->text_->h())/2);
    }
};