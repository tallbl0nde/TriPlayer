#include "ui/element/ListAlbumSong.hpp"

// Font size of all text
#define FONT_SIZE 22
// Height of item
#define HEIGHT 60
// Amount either side of screen to keep textures (in pixels)
#define TEX_THRESHOLD 2000
// Pixel gap between text fields
#define TEXT_GAP 20

namespace CustomElm {
    SDL_Texture * ListAlbumSong::lineTexture = nullptr;

    ListAlbumSong::ListAlbumSong() : Element(0, 0, 100, HEIGHT) {
        this->track = new Aether::Text(this->x(), this->y(), "", FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->track->setHidden(true);
        this->addElement(this->track);
        this->title = new Aether::Text(this->x(), this->y(), "", FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->title->setHidden(true);
        this->addElement(this->title);
        this->artist = new Aether::Text(this->x(), this->y(), "", FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->artist->setHidden(true);
        this->addElement(this->artist);
        this->length = new Aether::Text(this->x(), this->y(), "", FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->length->setHidden(true);
        this->addElement(this->length);
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

    void ListAlbumSong::setInactive() {
        Element::setInactive();
        this->callMore = false;
    }

    bool ListAlbumSong::handleEvent(Aether::InputEvent * e) {
        // Check if button press and focussed
        if (this->highlighted() && e->type() == Aether::EventType::ButtonPressed) {
            if (e->button() == Aether::Button::X) {
                this->moreCallback();
                return true;
            }
        }

        // Check if pressed over dots
        if (e->touchX() >= this->length->x() + this->length->w() && e->touchY() >= this->y() && e->touchX() <= this->x() + this->w() && e->touchY() <= this->y() + this->h()) {
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

    void ListAlbumSong::update(uint32_t dt) {
        Element::update(dt);

        switch (this->isRendering) {
            case Waiting:
                // Waiting to render - check position and start if within threshold
                if (this->y() + this->h() > -TEX_THRESHOLD && this->y() < 720 + TEX_THRESHOLD) {
                    this->track->startRendering();
                    this->title->startRendering();
                    this->artist->startRendering();
                    this->length->startRendering();
                    this->dots->startRendering();
                    this->isRendering = InProgress;
                }
                break;

            case InProgress:
                // Check if all are ready and if so move show and change to done
                if (this->track->textureReady() && this->title->textureReady() && this->artist->textureReady() && this->length->textureReady() && this->dots->textureReady()) {
                    this->positionItems();
                    this->track->setHidden(false);
                    this->title->setHidden(false);
                    this->artist->setHidden(false);
                    this->length->setHidden(false);
                    this->dots->setHidden(false);
                    this->isRendering = Done;
                }

            case Done:
                // Check if move outside of threshold and if so remove texture to save memory
                if (this->y() + this->h() < -TEX_THRESHOLD || this->y() > 720 + TEX_THRESHOLD) {
                    this->track->destroyTexture();
                    this->title->destroyTexture();
                    this->artist->destroyTexture();
                    this->length->destroyTexture();
                    this->dots->destroyTexture();
                    this->track->setHidden(true);
                    this->title->setHidden(true);
                    this->artist->setHidden(true);
                    this->length->setHidden(true);
                    this->dots->setHidden(true);
                    this->isRendering = Waiting;
                }
                break;
        }
    }

    void ListAlbumSong::render() {
        Element::render();
        if (this->isRendering == Done && this->isVisible()) {
            SDLHelper::drawTexture(this->lineTexture, this->lineColour, this->x(), this->y(), this->w());
            SDLHelper::drawTexture(this->lineTexture, this->lineColour, this->x(), this->y() + this->h(), this->w());
        }
    }

    void ListAlbumSong::setMoreCallback(std::function<void()> f) {
        this->moreCallback = f;
    }

    void ListAlbumSong::setTrackString(std::string s) {
        this->track->setString(s);
    }

    void ListAlbumSong::setTitleString(std::string s) {
        this->title->setString(s);
    }

    void ListAlbumSong::setArtistString(std::string s) {
        this->artist->setString(s);
    }

    void ListAlbumSong::setLengthString(std::string s) {
        this->length->setString(s);
    }

    void ListAlbumSong::setDotsColour(Aether::Colour c) {
        this->dots->setColour(c);
    }

    void ListAlbumSong::setLineColour(Aether::Colour c) {
        this->lineColour = c;
    }

    void ListAlbumSong::setTextColour(Aether::Colour c) {
        this->track->setColour(c);
        this->title->setColour(c);
        this->artist->setColour(c);
        this->length->setColour(c);
    }

    void ListAlbumSong::setW(int w) {
        Element::setW(w);
        this->positionItems();
    }

    void ListAlbumSong::positionItems() {
        this->track->setX(this->x() + 15);
        this->title->setX(this->x() + this->w() * 0.08);
        this->artist->setX(this->x() + this->w() * 0.6);
        this->length->setX(this->x() + this->w() - 50 - this->length->w());
        this->dots->setXY(this->x() + this->w() - 25, this->y() + (this->h() - this->dots->h())/2 + 1);
        this->track->setY(this->y() + (this->h() - this->track->h())/2);
        this->title->setY(this->y() + (this->h() - this->title->h())/2);
        this->artist->setY(this->y() + (this->h() - this->artist->h())/2);
        this->length->setY(this->y() + (this->h() - this->length->h())/2);

        if (this->track->x() + this->track->w() > this->title->x() - TEXT_GAP) {
            this->track->setW(this->title->x() - this->track->x() - TEXT_GAP);
        }
        if (!this->artist->string().empty()) {
            if (this->title->x() + this->title->w() > this->artist->x() - TEXT_GAP) {
                this->title->setW(this->artist->x() - this->title->x() - TEXT_GAP);
            }
            if (this->artist->x() + this->artist->w() > this->length->x() - TEXT_GAP) {
                this->artist->setW(this->length->x() - this->artist->x() - TEXT_GAP);
            }
        }
    }
}