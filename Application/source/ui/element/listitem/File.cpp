#include "ui/element/listitem/File.hpp"

// Font size
#define FONT_SIZE 24
// Height of item
#define HEIGHT 60
// Padding
#define PADDING 20

namespace CustomElm::ListItem {
    File::File(std::string s, bool b, std::function<void()> f) : Item(HEIGHT) {
        this->icon = new Aether::Image(0, 0, (b ? "romfs:/icons/directory.png" : "romfs:/icons/file.png"), 1, 1, Aether::RenderType::Deferred);
        this->watchTexture(this->icon);
        this->name = new Aether::Text(0, 0, s, FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->watchTexture(this->name);
        this->setCallback(f);
    }

    void File::positionItems() {
        this->icon->setXY(this->x() + PADDING, this->y() + (this->h() - this->icon->h())/2);
        this->name->setXY(this->icon->x() + this->icon->w() + PADDING, this->y() + (this->h() - this->name->h())/2);
        int maxW = this->x() + this->w() - PADDING - this->name->x();
        if (this->name->texW() > maxW) {
            this->name->setW(maxW);
        } else {
            this->name->setW(this->name->texW());
        }
    }

    void File::update(uint32_t dt) {
        Item::update(dt);

        // Scroll if this element is selected
        if (this->highlighted() && !this->name->scroll()) {
            this->name->setScroll(true);
        } else if (!this->highlighted() && this->name->scroll()) {
            this->name->setScroll(false);
        }
    }

    void File::setIconColour(Aether::Colour c) {
        this->icon->setColour(c);
    }

    void File::setTextColour(Aether::Colour c) {
        this->name->setColour(c);
    }
};