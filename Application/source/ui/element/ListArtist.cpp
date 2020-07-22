#include "ui/element/ListArtist.hpp"

// Font size of text
#define FONT_SIZE 24
// Height of element
#define HEIGHT 80
// Pixels of padding around image
#define PADDING 10
// Amount of pixels off screen before textures are discarded
#define TEX_THRESHOLD 500

namespace CustomElm {
    ListArtist::ListArtist(const std::string & path) : Aether::Element(0, 0, 100, HEIGHT) {
        this->image = new Aether::Image(0, 0, path, 1, 1, Aether::RenderType::Deferred);
        this->addElement(this->image);
        this->image->setHidden(true);
        this->name = new Aether::Text(0, 0, "", FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->name->setScrollSpeed(35);
        this->name->setScrollWaitTime(500);
        this->addElement(this->name);
        this->name->setHidden(true);
        this->isRendering = Waiting;
    }

    void ListArtist::update(uint32_t dt) {
        Element::update(dt);

        // Scroll if this element is selected
        if (this->highlighted() && !this->name->scroll()) {
            this->name->setScroll(true);
        } else if (!this->highlighted() && this->name->scroll()) {
            this->name->setScroll(false);
        }

        switch (this->isRendering) {
            case Waiting:
                // Waiting to render - check position and start if within threshold
                if (this->y() + this->h() > -TEX_THRESHOLD && this->y() < 720 + TEX_THRESHOLD) {
                    this->image->startRendering();
                    this->name->startRendering();
                    this->isRendering = InProgress;
                }
                break;

            case InProgress:
                // Check if all are ready and if so move show and change to done
                if (this->image->textureReady() && this->name->textureReady()) {
                    this->positionItems();
                    this->image->setHidden(false);
                    this->name->setHidden(false);
                    this->isRendering = Done;
                }

            case Done:
                // Check if move outside of threshold and if so remove texture to save memory
                if (this->y() + this->h() < -TEX_THRESHOLD || this->y() > 720 + TEX_THRESHOLD) {
                    this->image->destroyTexture();
                    this->name->destroyTexture();
                    this->image->setHidden(true);
                    this->name->setHidden(true);
                    this->isRendering = Waiting;
                }
                break;
        }
    }

    void ListArtist::setName(const std::string & s) {
        this->name->setString(s);
    }

    void ListArtist::setTextColour(Aether::Colour c) {
        this->name->setColour(c);
    }

    void ListArtist::positionItems() {
        this->image->setXY(this->x() + PADDING, this->y() + PADDING);
        this->image->setWH(HEIGHT - 2*PADDING, HEIGHT - 2*PADDING);
        this->name->setX(this->image->x() + this->image->w() + 2*PADDING);
        this->name->setY(this->y() + (this->h() - this->name->h())/2);
        int maxW = this->x() + this->w() - this->name->x() - 2*PADDING;
        if (this->name->w() > maxW) {
            this->name->setW(maxW);
        }
    }
};