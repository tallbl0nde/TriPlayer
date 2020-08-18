#include "ui/element/ListHeadingCount.hpp"

// Font sizes
#define HEADING_SIZE 32
#define COUNT_SIZE 20
// Text gap
#define GAP 20

namespace CustomElm {
    ListHeadingCount::ListHeadingCount() : Aether::Element() {
        // Create and position elements
        this->heading = new Aether::Text(this->x(), this->y(), "Py", HEADING_SIZE);
        this->setH(this->heading->h());
        this->addElement(this->heading);
        this->count = new Aether::Text(this->x(), 0, "(0)", COUNT_SIZE);
        this->count->setY(this->y() + this->h() - this->count->h() - 5);
        this->addElement(this->count);
    }

    void ListHeadingCount::setHeadingString(const std::string & str) {
        this->heading->setString(str);
        this->count->setX(this->heading->x() + this->heading->w() + GAP);
    }

    void ListHeadingCount::setCount(const size_t num) {
        this->count->setString("(" + std::to_string(num) + ")");
    }

    void ListHeadingCount::setTextColour(const Aether::Colour & c) {
        this->heading->setColour(c);
        this->count->setColour(c);
    }
};