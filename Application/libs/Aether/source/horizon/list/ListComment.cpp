#include "ListComment.hpp"

// Font size
#define FONT_SIZE 16
// x and y padding
#define PADDING 18

namespace Aether {
    ListComment::ListComment(std::string s) : Element() {
        this->text = new TextBlock(PADDING, PADDING, s, FONT_SIZE, this->w() - 2*PADDING);
        this->addElement(this->text);
    }

    void ListComment::updateElement() {
        this->text->setWrapWidth(this->w() - 2*PADDING);
        this->setH(this->text->h() + 2*PADDING);
    }

    Colour ListComment::getTextColour() {
        return this->text->getColour();
    }

    void ListComment::setTextColour(Colour c) {
        this->text->setColour(c);
    }

    void ListComment::setW(int w) {
        Element::setW(w);
        this->updateElement();
    }
};