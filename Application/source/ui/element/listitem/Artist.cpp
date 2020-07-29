#include "ui/element/listitem/Artist.hpp"

// Font size of text
#define FONT_SIZE 24
// Height of element
#define HEIGHT 80
// Pixels of padding around image
#define PADDING 10

namespace CustomElm::ListItem {
    Artist::Artist(const std::string & path) : Item(HEIGHT) {
        this->image = new Aether::Image(0, 0, path, 1, 1, Aether::RenderType::Deferred);
        this->watchTexture(this->image);
        this->name = new Aether::Text(0, 0, "", FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->name->setScrollSpeed(35);
        this->name->setScrollWaitTime(500);
        this->watchTexture(this->name);
    }

    void Artist::positionItems() {
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
        if (this->highlighted() && !this->name->scroll()) {
            this->name->setScroll(true);
        } else if (!this->highlighted() && this->name->scroll()) {
            this->name->setScroll(false);
        }
    }

    void Artist::setName(const std::string & s) {
        this->name->setString(s);
    }

    void Artist::setTextColour(Aether::Colour c) {
        this->name->setColour(c);
    }
};