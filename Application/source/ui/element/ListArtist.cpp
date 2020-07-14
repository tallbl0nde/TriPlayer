#include "ui/element/ListArtist.hpp"

// Font sizes
#define COUNT_FONT_SIZE 18
#define NAME_FONT_SIZE 24
// Dimensions
#define IMAGE_SIZE 150
#define WIDTH 250
#define HEIGHT 230
// Amount either side of screen to keep textures (in pixels)
#define TEX_THRESHOLD 2000
// Padding either side of name
#define SIDE_PADDING 35

namespace CustomElm {
    ListArtist::ListArtist(std::string path) : Element(0, 0, WIDTH, HEIGHT) {
        this->image = new Aether::Image(this->x(), this->y(), path, 1, 1, Aether::RenderType::Deferred);
        this->addElement(this->image);
        this->image->setHidden(true);
        this->name = new Aether::Text(this->x(), this->y(), "", NAME_FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->name->setHidden(true);
        this->name->setScrollSpeed(35);
        this->name->setScrollWaitTime(500);
        this->addElement(this->name);
        this->counts = new Aether::Text(this->x(), this->y(), "", COUNT_FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->counts->setHidden(true);
        this->counts->setScrollSpeed(35);
        this->counts->setScrollWaitTime(500);
        this->addElement(this->counts);
        this->dots = new Aether::Image(this->x(), this->y(), "romfs:/icons/verticaldots.png", 1, 1, Aether::RenderType::Deferred);
        this->addElement(this->dots);
        this->dots->setHidden(true);
        this->isRendering = Waiting;
    }

    void ListArtist::setInactive() {
        Element::setInactive();
        this->callMore = false;
    }

    void ListArtist::setW(int w) {
        // Do nothing
    }

    bool ListArtist::handleEvent(Aether::InputEvent * e) {
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

    void ListArtist::update(uint32_t dt) {
        Element::update(dt);

        // Scroll if this element is selected
        if (this->highlighted() && !this->name->scroll()) {
            this->name->setScroll(true);
            this->counts->setScroll(true);
        } else if (!this->highlighted() && this->name->scroll()) {
            this->name->setScroll(false);
            this->counts->setScroll(false);
        }

        switch (this->isRendering) {
            case Waiting:
                // Waiting to render - check position and start if within threshold
                if (this->y() + this->h() > -TEX_THRESHOLD && this->y() < 720 + TEX_THRESHOLD) {
                    this->image->startRendering();
                    this->name->startRendering();
                    this->counts->startRendering();
                    this->dots->startRendering();
                    this->isRendering = InProgress;
                }
                break;

            case InProgress:
                // Check if all are ready and if so move show and change to done
                if (this->image->textureReady() && this->name->textureReady() && this->counts->textureReady() && this->dots->textureReady()) {
                    this->positionItems();
                    this->image->setHidden(false);
                    this->name->setHidden(false);
                    this->counts->setHidden(false);
                    this->dots->setHidden(false);
                    this->isRendering = Done;
                }

            case Done:
                // Check if move outside of threshold and if so remove texture to save memory
                if (this->y() + this->h() < -TEX_THRESHOLD || this->y() > 720 + TEX_THRESHOLD) {
                    this->image->destroyTexture();
                    this->name->destroyTexture();
                    this->counts->destroyTexture();
                    this->dots->destroyTexture();
                    this->image->setHidden(true);
                    this->name->setHidden(true);
                    this->counts->setHidden(true);
                    this->dots->setHidden(true);
                    this->isRendering = Waiting;
                }
                break;
        }
    }

    void ListArtist::setMoreCallback(std::function<void()> f) {
        this->moreCallback = f;
    }

    void ListArtist::setNameString(std::string s) {
        this->name->setString(s);
    }

    void ListArtist::setCountsString(std::string s) {
        this->counts->setString(s);
    }

    void ListArtist::setDotsColour(Aether::Colour c) {
        this->dots->setColour(c);
    }

    void ListArtist::setTextColour(Aether::Colour c) {
        this->name->setColour(c);
    }

    void ListArtist::setMutedTextColour(Aether::Colour c) {
        this->counts->setColour(c);
    }

    void ListArtist::positionItems() {
        this->image->setXY(this->x() + (this->w() - IMAGE_SIZE)/2, this->y() + 5);
        this->image->setWH(IMAGE_SIZE, IMAGE_SIZE);

        // Make text scrollable if it's too long
        this->name->setY(this->image->y() + this->image->h() + 5);
        if (this->name->w() > this->w() - 2*SIDE_PADDING) {
            this->name->setX(this->x() + SIDE_PADDING);
            this->name->setW(this->w() - 2*SIDE_PADDING);
        } else {
            this->name->setX(this->x() + (this->w() - this->name->w())/2);
        }
        this->counts->setY(this->name->y() + this->name->h() + 4);
        if (this->counts->w() > this->w() - 2*SIDE_PADDING) {
            this->counts->setX(this->x() + SIDE_PADDING);
            this->counts->setW(this->w() - 2*SIDE_PADDING);
        } else {
            this->counts->setX(this->x() + (this->w() - this->counts->w())/2);
        }

        this->dots->setXY(this->x() + this->w() - 20, this->counts->y() - 2 - (this->dots->h())/2);
    }
}