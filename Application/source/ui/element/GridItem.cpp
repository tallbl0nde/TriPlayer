#include "ui/element/GridItem.hpp"

// Font sizes
#define MAIN_FONT_SIZE 18
#define SUB_FONT_SIZE 24
// Dimensions
#define IMAGE_SIZE 150
#define WIDTH 250
#define HEIGHT 230
// Amount either side of screen to keep textures (in pixels)
#define TEX_THRESHOLD 2000
// Padding either side of name
#define SIDE_PADDING 35

namespace CustomElm {
    GridItem::GridItem(std::string path) : Element(0, 0, WIDTH, HEIGHT) {
        this->image = new Aether::Image(this->x(), this->y(), path, 1, 1, Aether::RenderType::Deferred);
        this->addElement(this->image);
        this->image->setHidden(true);
        this->main = new Aether::Text(this->x(), this->y(), "", SUB_FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->main->setHidden(true);
        this->main->setScrollSpeed(35);
        this->main->setScrollWaitTime(500);
        this->addElement(this->main);
        this->sub = new Aether::Text(this->x(), this->y(), "", MAIN_FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->sub->setHidden(true);
        this->sub->setScrollSpeed(35);
        this->sub->setScrollWaitTime(500);
        this->addElement(this->sub);
        this->dots = new Aether::Image(this->x(), this->y(), "romfs:/icons/verticaldots.png", 1, 1, Aether::RenderType::Deferred);
        this->addElement(this->dots);
        this->dots->setHidden(true);
        this->isRendering = Waiting;
    }

    void GridItem::setInactive() {
        Element::setInactive();
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

        return Element::handleEvent(e);
    }

    void GridItem::update(uint32_t dt) {
        Element::update(dt);

        // Scroll if this element is selected
        if (this->highlighted() && !this->main->scroll()) {
            this->main->setScroll(true);
            this->sub->setScroll(true);
        } else if (!this->highlighted() && this->main->scroll()) {
            this->main->setScroll(false);
            this->sub->setScroll(false);
        }

        switch (this->isRendering) {
            case Waiting:
                // Waiting to render - check position and start if within threshold
                if (this->x() > -TEX_THRESHOLD && this->x() + this->w() < 1280 + TEX_THRESHOLD && this->y() + this->h() > -TEX_THRESHOLD && this->y() < 720 + TEX_THRESHOLD) {
                    this->image->startRendering();
                    this->main->startRendering();
                    this->sub->startRendering();
                    this->dots->startRendering();
                    this->isRendering = InProgress;
                }
                break;

            case InProgress:
                // Check if all are ready and if so move show and change to done
                if (this->image->textureReady() && this->main->textureReady() && this->sub->textureReady() && this->dots->textureReady()) {
                    this->positionItems();
                    this->image->setHidden(false);
                    this->main->setHidden(false);
                    this->sub->setHidden(false);
                    this->dots->setHidden(false);
                    this->isRendering = Done;
                }

            case Done:
                // Check if move outside of threshold and if so remove texture to save memory
                if (this->x() < -TEX_THRESHOLD || this->x() + this->w() > 1280 + TEX_THRESHOLD || this->y() + this->h() < -TEX_THRESHOLD || this->y() > 720 + TEX_THRESHOLD) {
                    this->image->destroyTexture();
                    this->main->destroyTexture();
                    this->sub->destroyTexture();
                    this->dots->destroyTexture();
                    this->image->setHidden(true);
                    this->main->setHidden(true);
                    this->sub->setHidden(true);
                    this->dots->setHidden(true);
                    this->isRendering = Waiting;
                }
                break;
        }
    }

    void GridItem::setMoreCallback(std::function<void()> f) {
        this->moreCallback = f;
    }

    void GridItem::setMainString(std::string s) {
        this->main->setString(s);
    }

    void GridItem::setSubString(std::string s) {
        this->sub->setString(s);
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

    void GridItem::positionItems() {
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