#include "ui/element/GridItem.hpp"

// Font sizes
#define MAIN_FONT_SIZE 24
#define SUB_FONT_SIZE 18
// Dimensions
#define IMAGE_SIZE 150
#define WIDTH 250
#define HEIGHT 230
// Padding either side of name
#define SIDE_PADDING 35

namespace CustomElm {
    GridItem::GridItem(std::string path) : AsyncItem() {
        // Need to call base so our overridden method isn't called
        AsyncItem::setW(WIDTH);
        AsyncItem::setH(HEIGHT);

        // Create elements
        this->image = new Aether::Image(this->x(), this->y(), path, Aether::Render::Wait);
        this->addElement(this->image);
        this->addTexture(this->image);

        this->main = new Aether::Text(this->x(), this->y(), "", SUB_FONT_SIZE, Aether::Render::Wait);
        this->sub = new Aether::Text(this->x(), this->y(), "", MAIN_FONT_SIZE, Aether::Render::Wait);

        this->dots = new Aether::Image(this->x(), this->y(), "romfs:/icons/verticaldots.png", Aether::Render::Wait);
        this->addElement(this->dots);
        this->addTexture(this->dots);
    }

    void GridItem::setInactive() {
        AsyncItem::setInactive();
        this->callMore = false;
    }

    void GridItem::setW(int w) {
        // Do nothing
    }

    bool GridItem::handleEvent(Aether::InputEvent * e) {
        // Check if button press and focussed
        if (this->highlighted() && e->type() == Aether::EventType::ButtonPressed) {
            if (e->button() == Aether::Button::X) {
                this->moreCallback();
                return true;
            }
        }

        // Check if pressed over dots
        if (e->touchX() >= this->dots->x() - 15 && e->touchY() >= this->dots->y() - 20 && e->touchX() <= this->x() + this->w() && e->touchY() <= this->y() + this->h()) {
            if (e->type() == Aether::EventType::TouchPressed) {
                // Do nothing when dots pressed
                this->callMore = true;
                return true;

            } else if (e->type() == Aether::EventType::TouchReleased && this->callMore) {
                this->moreCallback();
                this->callMore = false;
                return true;
            }
        }

        return AsyncItem::handleEvent(e);
    }

    void GridItem::processText(Aether::Text * & text, std::function<Aether::Text * ()> getNew) {
        // Remove original
        this->removeTexture(text);
        this->removeElement(text);

        // Get (and assign) new text object
        text = getNew();

        // Don't add if empty string
        if (!text->string().empty()) {
            this->addElement(text);
            this->addTexture(text);
        }
    }

    void GridItem::update(uint32_t dt) {
        AsyncItem::update(dt);

        // Scroll if this element is selected
        if (this->highlighted() && !this->main->canScroll()) {
            this->main->setCanScroll(true);
            this->sub->setCanScroll(true);
        } else if (!this->highlighted() && this->main->canScroll()) {
            this->main->setCanScroll(false);
            this->sub->setCanScroll(false);
        }
    }

    void GridItem::setMoreCallback(std::function<void()> f) {
        this->moreCallback = f;
    }

    void GridItem::setMainString(std::string s) {
        this->processText(this->main, [this, s]() -> Aether::Text * {
            Aether::Text * t = new Aether::Text(this->x(), this->y(), s, MAIN_FONT_SIZE, Aether::Render::Wait);
            t->setScrollPause(500);
            t->setScrollSpeed(35);
            return t;
        });
    }

    void GridItem::setSubString(std::string s) {
        this->processText(this->sub, [this, s]() -> Aether::Text * {
            Aether::Text * t = new Aether::Text(this->x(), this->y(), s, SUB_FONT_SIZE, Aether::Render::Wait);
            t->setScrollPause(500);
            t->setScrollSpeed(35);
            return t;
        });
    }

    void GridItem::setDotsColour(Aether::Colour c) {
        this->dots->setColour(c);
    }

    void GridItem::setTextColour(Aether::Colour c) {
        this->main->setColour(c);
    }

    void GridItem::setMutedTextColour(Aether::Colour c) {
        this->sub->setColour(c);
    }

    void GridItem::positionElements() {
        this->image->setXY(this->x() + (this->w() - IMAGE_SIZE)/2, this->y() + 5);
        this->image->setWH(IMAGE_SIZE, IMAGE_SIZE);

        // Make text scrollable if it's too long
        this->main->setY(this->image->y() + this->image->h() + 5);
        if (this->main->w() > this->w() - 2*SIDE_PADDING) {
            this->main->setX(this->x() + SIDE_PADDING);
            this->main->setW(this->w() - 2*SIDE_PADDING);
        } else {
            this->main->setX(this->x() + (this->w() - this->main->w())/2);
        }
        this->sub->setY(this->main->y() + this->main->h() + 4);
        if (this->sub->w() > this->w() - 2*SIDE_PADDING) {
            this->sub->setX(this->x() + SIDE_PADDING);
            this->sub->setW(this->w() - 2*SIDE_PADDING);
        } else {
            this->sub->setX(this->x() + (this->w() - this->sub->w())/2);
        }

        this->dots->setXY(this->x() + this->w() - 20, this->sub->y() - 2 - (this->dots->h())/2);
    }
}
