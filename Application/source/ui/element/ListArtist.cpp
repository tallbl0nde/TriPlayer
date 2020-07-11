#include "ui/element/ListArtist.hpp"

// Font sizes
#define COUNT_FONT_SIZE 18
#define NAME_FONT_SIZE 24
// Height of item
#define HEIGHT 80
// Amount either side of list to keep textures (in pixels)
#define TEX_THRESHOLD 2000
// Pixel gap between text fields
#define TEXT_GAP 30

namespace CustomElm {
    SDL_Texture * ListArtist::lineTexture = nullptr;

    ListArtist::ListArtist() : Element(0, 0, 100, HEIGHT) {
        this->name = new Aether::Text(this->x(), this->y(), "", NAME_FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->name->setHidden(true);
        this->addElement(this->name);
        this->counts = new Aether::Text(this->x(), this->y(), "", COUNT_FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->counts->setHidden(true);
        this->addElement(this->counts);
        this->dots = new Aether::Image(this->x(), this->y(), "romfs:/icons/verticaldots.png", 1, 1, Aether::RenderType::Deferred);
        this->addElement(this->dots);
        this->dots->setHidden(true);
        this->isRendering = Waiting;

        // Create line texture if it doesn't exist
        if (this->lineTexture == nullptr) {
            this->lineTexture = SDLHelper::renderFilledRect(this->w(), 1);
        }
        this->lineColour = Aether::Colour{255, 255, 255, 255};
    }

    void ListArtist::setInactive() {
        Element::setInactive();
        this->callMore = false;
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
        // if (e->touchX() >= this->length->x() + this->length->w() && e->touchY() >= this->y() && e->touchX() <= this->x() + this->w() && e->touchY() <= this->y() + this->h()) {
        //     if (e->type() == Aether::EventType::TouchPressed) {
        //         // Do nothing when dots pressed
        //         this->callMore = true;
        //         return true;

        //     } else if (e->type() == Aether::EventType::TouchReleased && this->callMore) {
        //         this->moreCallback();
        //         this->callMore = false;
        //         return true;
        //     }
        // }

        return Element::handleEvent(e);
    }

    void ListArtist::update(uint32_t dt) {
        Element::update(dt);

        switch (this->isRendering) {
            case Waiting:
                // Waiting to render - check position and start if within threshold
                if (this->y() >= this->parent()->y() - TEX_THRESHOLD && this->y() <= this->parent()->y() + this->parent()->h() + TEX_THRESHOLD) {
                    this->name->startRendering();
                    this->counts->startRendering();
                    this->dots->startRendering();
                    this->isRendering = InProgress;
                }
                break;

            case InProgress:
                // Check if all are ready and if so move show and change to done
                if (this->name->textureReady() && this->counts->textureReady() && this->dots->textureReady()) {
                    this->positionItems();
                    this->name->setHidden(false);
                    this->counts->setHidden(false);
                    this->dots->setHidden(false);
                    this->isRendering = Done;
                }

            case Done:
                // Check if move outside of threshold and if so remove texture to save memory
                if (this->y() < this->parent()->y() - TEX_THRESHOLD || this->y() > this->parent()->y() + this->parent()->h() + TEX_THRESHOLD) {
                    this->name->destroyTexture();
                    this->counts->destroyTexture();
                    this->dots->destroyTexture();
                    this->name->setHidden(true);
                    this->counts->setHidden(true);
                    this->dots->setHidden(true);
                    this->isRendering = Waiting;
                }
                break;
        }
    }

    void ListArtist::render() {
        Element::render();
        if (this->isRendering == Done && this->isVisible()) {
            SDLHelper::drawTexture(this->lineTexture, this->lineColour, this->x(), this->y(), this->w());
            SDLHelper::drawTexture(this->lineTexture, this->lineColour, this->x(), this->y() + this->h(), this->w());
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

    void ListArtist::setLineColour(Aether::Colour c) {
        this->lineColour = c;
    }

    void ListArtist::setTextColour(Aether::Colour c) {
        this->name->setColour(c);
    }

    void ListArtist::setMutedTextColour(Aether::Colour c) {
        this->counts->setColour(c);
    }

    void ListArtist::setW(int w) {
        Element::setW(w);
        this->positionItems();
    }

    void ListArtist::positionItems() {
        this->name->setX(this->x() + 15);
        this->counts->setX(this->name->x());
        this->dots->setXY(this->x() + this->w() - 25, this->y() + (this->h() - this->dots->h())/2 + 1);
        this->name->setY(this->y() + 11);
        this->counts->setY(this->name->y() + this->name->h() + 5);
    }
}