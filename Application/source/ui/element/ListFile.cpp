#include "ui/element/ListFile.hpp"

// Font size
#define FONT_SIZE 24
// Height of item
#define HEIGHT 60
// Padding
#define PADDING 20
#define TEX_THRESHOLD 500

namespace CustomElm {
    ListFile::ListFile(std::string s, bool b, std::function<void()> f) : Aether::Element(0, 0, 100, HEIGHT) {
        this->icon = new Aether::Image(0, 0, (b ? "romfs:/icons/directory.png" : "romfs:/icons/file.png"), 1, 1, Aether::RenderType::Deferred);
        this->addElement(this->icon);
        this->icon->setHidden(true);
        this->name = new Aether::Text(0, 0, s, FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->addElement(this->name);
        this->name->setHidden(true);
        this->positionItems();
        this->setCallback(f);
        this->isRendering = RenderingStatus::Waiting;
    }

    void ListFile::positionItems() {
        this->icon->setXY(this->x() + PADDING, this->y() + (this->h() - this->icon->h())/2);
        this->name->setXY(this->icon->x() + this->icon->w() + PADDING, this->y() + (this->h() - this->name->h())/2);
        int maxW = this->x() + this->w() - PADDING - this->name->x();
        if (this->name->texW() > maxW) {
            this->name->setW(maxW);
        } else {
            this->name->setW(this->name->texW());
        }
    }

    void ListFile::setIconColour(Aether::Colour c) {
        this->icon->setColour(c);
    }

    void ListFile::setTextColour(Aether::Colour c) {
        this->name->setColour(c);
    }

    void ListFile::update(uint32_t dt) {
        Element::update(dt);

        // Scroll if this element is selected
        if (this->highlighted() && !this->name->scroll()) {
            this->name->setScroll(true);
        } else if (!this->highlighted() && this->name->scroll()) {
            this->name->setScroll(false);
        }

        // Render elements if close enough
        switch (this->isRendering) {
            case Waiting:
                // Waiting to render - check position and start if within threshold
                if (this->y() + this->h() > -TEX_THRESHOLD && this->y() < 720 + TEX_THRESHOLD) {
                    this->icon->startRendering();
                    this->name->startRendering();
                    this->isRendering = InProgress;
                }
                break;

            case InProgress:
                // Check if all are ready and if so move show and change to done
                if (this->icon->textureReady() && this->name->textureReady()) {
                    this->positionItems();
                    this->icon->setHidden(false);
                    this->name->setHidden(false);
                    this->isRendering = Done;
                }

            case Done:
                // Check if move outside of threshold and if so remove texture to save memory
                if (this->y() + this->h() < -TEX_THRESHOLD || this->y() > 720 + TEX_THRESHOLD) {
                    this->icon->destroyTexture();
                    this->name->destroyTexture();
                    this->icon->setHidden(true);
                    this->name->setHidden(true);
                    this->isRendering = Waiting;
                }
                break;
        }
    }

    void ListFile::setW(int w) {
        Element::setW(w);
    }
};