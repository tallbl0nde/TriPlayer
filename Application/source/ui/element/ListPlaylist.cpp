#include "ui/element/ListPlaylist.hpp"

// Font sizes
#define NAME_FONT_SIZE 28
#define SONGS_FONT_SIZE 20
// Height of item
#define HEIGHT 100
// Padding around image
#define PADDING 12
// Amount either side of list to keep textures (in pixels)
#define TEX_THRESHOLD 1000

namespace CustomElm {
    SDL_Texture * ListPlaylist::lineTexture = nullptr;

    ListPlaylist::ListPlaylist(const std::string & img) : Element(0, 0, 100, HEIGHT) {
        this->image = new Aether::Image(this->x() + PADDING, this->y() + PADDING, img, 1, 1, Aether::RenderType::Deferred);
        this->image->setHidden(true);
        this->addElement(this->image);
        this->name = new Aether::Text(this->x(), this->y(), "", NAME_FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->name->setHidden(true);
        this->addElement(this->name);
        this->songs = new Aether::Text(this->x(), this->y(), "", SONGS_FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->songs->setHidden(true);
        this->addElement(this->songs);
        this->dots = new Aether::Image(this->x(), this->y(), "romfs:/icons/verticaldots.png", 1, 1, Aether::RenderType::Deferred);
        this->dots->setHidden(true);
        this->addElement(this->dots);
        this->isRendering = Waiting;

        // Create line texture if it doesn't exist
        if (this->lineTexture == nullptr) {
            this->lineTexture = SDLHelper::renderFilledRect(this->w(), 1);
        }
        this->lineColour = Aether::Colour{255, 255, 255, 255};
    }

    void ListPlaylist::setInactive() {
        Element::setInactive();
        this->callMore = false;
    }

    bool ListPlaylist::handleEvent(Aether::InputEvent * e) {
        // Check if button press and focussed
        if (this->highlighted() && e->type() == Aether::EventType::ButtonPressed) {
            if (e->button() == Aether::Button::X) {
                this->moreCallback();
                return true;
            }
        }

        // Check if pressed over dots
        if (e->touchX() >= this->dots->x() - 20 && e->touchY() >= this->y() && e->touchX() <= this->dots->x() + this->dots->w() + 20 && e->touchY() <= this->y() + this->h()) {
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

    void ListPlaylist::update(uint32_t dt) {
        Element::update(dt);

        switch (this->isRendering) {
            case Waiting:
                // Waiting to render - check position and start if within threshold
                if (this->y() >= this->parent()->y() - TEX_THRESHOLD && this->y() <= this->parent()->y() + this->parent()->h() + TEX_THRESHOLD) {
                    this->image->startRendering();
                    this->name->startRendering();
                    this->songs->startRendering();
                    this->dots->startRendering();
                    this->isRendering = InProgress;
                }
                break;

            case InProgress:
                // Check if all are ready and if so move show and change to done
                if (this->image->textureReady() && this->name->textureReady() && this->songs->textureReady() && this->dots->textureReady()) {
                    this->positionItems();
                    this->image->setHidden(false);
                    this->name->setHidden(false);
                    this->songs->setHidden(false);
                    this->dots->setHidden(false);
                    this->isRendering = Done;
                }

            case Done:
                // Check if move outside of threshold and if so remove texture to save memory
                if (this->y() < this->parent()->y() - TEX_THRESHOLD || this->y() > this->parent()->y() + this->parent()->h() + TEX_THRESHOLD) {
                    this->image->destroyTexture();
                    this->name->destroyTexture();
                    this->songs->destroyTexture();
                    this->dots->destroyTexture();
                    this->image->setHidden(true);
                    this->name->setHidden(true);
                    this->songs->setHidden(true);
                    this->dots->setHidden(true);
                    this->isRendering = Waiting;
                }
                break;
        }
    }

    void ListPlaylist::render() {
        Element::render();
        if (this->isRendering == Done && this->isVisible()) {
            SDLHelper::drawTexture(this->lineTexture, this->lineColour, this->x(), this->y(), this->w());
            SDLHelper::drawTexture(this->lineTexture, this->lineColour, this->x(), this->y() + this->h(), this->w());
        }
    }

    void ListPlaylist::setMoreCallback(std::function<void()> f) {
        this->moreCallback = f;
    }

    void ListPlaylist::setNameString(const std::string & s) {
        this->name->setString(s);
    }

    void ListPlaylist::setSongsString(const std::string & s) {
        this->songs->setString(s);
    }

    void ListPlaylist::setDotsColour(Aether::Colour c) {
        this->dots->setColour(c);
    }

    void ListPlaylist::setLineColour(Aether::Colour c) {
        this->lineColour = c;
    }

    void ListPlaylist::setTextColour(Aether::Colour c) {
        this->name->setColour(c);
    }

    void ListPlaylist::setMutedTextColour(Aether::Colour c) {
        this->songs->setColour(c);
    }

    void ListPlaylist::setW(int w) {
        Element::setW(w);
        this->positionItems();
    }

    void ListPlaylist::positionItems() {
        this->image->setWH(HEIGHT - 2*PADDING, HEIGHT - 2*PADDING);
        this->name->setX(this->image->x() + this->image->w() + 2*PADDING);
        this->name->setY(this->y() + 0.38*HEIGHT - this->name->h()/2);
        this->songs->setX(this->name->x());
        this->songs->setY(this->y() + 0.55*HEIGHT);
        this->dots->setXY(this->x() + this->w() - 25, this->y() + (this->h() - this->dots->h())/2 + 1);
    }
}