#include "ui/element/listitem/Artist.hpp"

// Font size of text
#define FONT_SIZE 24
// Height of element
#define HEIGHT 80
// Pixels of padding around image
#define PADDING 10

namespace CustomElm::ListItem {
    Artist::Artist(const std::string & path) : Item(HEIGHT) {
        this->image = new Aether::Image(0, 0, path, Aether::Render::Wait);
        this->addElement(this->image);
        this->addTexture(this->image);
        this->name = new Aether::Text(0, 0, "", FONT_SIZE, Aether::Render::Wait);
        this->name->setScrollPause(500);
        this->name->setScrollSpeed(35);
        this->addElement(this->name);
        this->addTexture(this->name);
    }

    void Artist::positionElements() {
        this->image->setXY(this->x() + PADDING, this->y() + PADDING);
        this->image->setWH(HEIGHT - 2*PADDING, HEIGHT - 2*PADDING);
        this->name->setX(this->image->x() + this->image->w() + 2*PADDING);
        this->name->setY(this->y() + (this->h() - this->name->h())/2);
        int maxW = this->x() + this->w() - this->name->x() - 2*PADDING;
        if (this->name->w() > maxW) {
            this->name->setW(maxW);
        }
    }

    void Artist::update(uint32_t dt) {
        Item::update(dt);

        // Scroll if this element is selected
        if (this->highlighted() && !this->name->canScroll()) {
            this->name->setCanScroll(true);
        } else if (!this->highlighted() && this->name->canScroll()) {
            this->name->setCanScroll(false);
        }
    }

    void Artist::setName(const std::string & s) {
        this->processText(this->name, [s]() -> Aether::Text * {
            return new Aether::Text(0, 0, s, FONT_SIZE, Aether::Render::Wait);
        });
    }

    void Artist::setTextColour(Aether::Colour c) {
        this->name->setColour(c);
    }
};
