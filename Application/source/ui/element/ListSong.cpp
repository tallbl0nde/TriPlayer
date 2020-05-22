#include "ListSong.hpp"

// Font size of all text
#define FONT_SIZE 22
// Height of item
#define HEIGHT 60
// Amount either side of list to keep textures (in pixels)
#define TEX_THRESHOLD 2000
// Pixel gap between text fields
#define TEXT_GAP 30

namespace CustomElm {
    SDL_Texture * ListSong::lineTexture = nullptr;

    ListSong::ListSong() : Element(0, 0, 100, HEIGHT) {
        this->title = new Aether::Text(this->x(), this->y(), "", FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->title->setHidden(true);
        this->addElement(this->title);
        this->artist = new Aether::Text(this->x(), this->y(), "", FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->artist->setHidden(true);
        this->addElement(this->artist);
        this->album = new Aether::Text(this->x(), this->y(), "", FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->album->setHidden(true);
        this->addElement(this->album);
        this->length = new Aether::Text(this->x(), this->y(), "", FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->length->setHidden(true);
        this->addElement(this->length);
        this->isRendering = Waiting;

        // Create line texture if it doesn't exist
        if (this->lineTexture == nullptr) {
            this->lineTexture = SDLHelper::renderFilledRect(this->w(), 1);
        }
        this->lineColour = Aether::Colour{255, 255, 255, 255};
    }

    void ListSong::update(uint32_t dt) {
        Element::update(dt);

        switch (this->isRendering) {
            case Waiting:
                // Waiting to render - check position and start if within threshold
                if (this->y() >= this->parent()->y() - TEX_THRESHOLD && this->y() <= this->parent()->y() + this->parent()->h() + TEX_THRESHOLD) {
                    this->title->startRendering();
                    this->artist->startRendering();
                    this->album->startRendering();
                    this->length->startRendering();
                    this->isRendering = InProgress;
                }
                break;

            case InProgress:
                // Check if all are ready and if so move show and change to done
                if (this->title->textureReady() && this->artist->textureReady() && this->album->textureReady() && this->length->textureReady()) {
                    this->positionItems();
                    this->title->setHidden(false);
                    this->artist->setHidden(false);
                    this->album->setHidden(false);
                    this->length->setHidden(false);
                    this->isRendering = Done;
                }

            case Done:
                // Check if move outside of threshold and if so remove texture to save memory
                if (this->y() < this->parent()->y() - TEX_THRESHOLD || this->y() > this->parent()->y() + this->parent()->h() + TEX_THRESHOLD) {
                    this->title->destroyTexture();
                    this->artist->destroyTexture();
                    this->album->destroyTexture();
                    this->length->destroyTexture();
                    this->title->setHidden(true);
                    this->artist->setHidden(true);
                    this->album->setHidden(true);
                    this->length->setHidden(true);
                    this->isRendering = Waiting;
                }
                break;
        }
    }

    void ListSong::render() {
        Element::render();
        if (this->isRendering == Done && this->isVisible()) {
            SDLHelper::drawTexture(this->lineTexture, this->lineColour, this->x(), this->y(), this->w());
            SDLHelper::drawTexture(this->lineTexture, this->lineColour, this->x(), this->y() + this->h(), this->w());
        }
    }

    void ListSong::restartRendering() {
        // Clear elements if they do not have a texture
        if (!this->title->textureReady()) {
            this->title->destroyTexture();
        }
        if (!this->artist->textureReady()) {
            this->artist->destroyTexture();
        }
        if (!this->album->textureReady()) {
            this->album->destroyTexture();
        }
        if (!this->length->textureReady()) {
            this->length->destroyTexture();
        }

        // Go back to waiting stage
        this->setHidden(true);
        this->isRendering = Waiting;
    }

    void ListSong::setTitleString(std::string s) {
        this->title->setString(s);
    }

    void ListSong::setArtistString(std::string s) {
        this->artist->setString(s);
    }

    void ListSong::setAlbumString(std::string s) {
        this->album->setString(s);
    }

    void ListSong::setLengthString(std::string s) {
        this->length->setString(s);
    }

    void ListSong::setLineColour(Aether::Colour c) {
        this->lineColour = c;
    }

    void ListSong::setTextColour(Aether::Colour c) {
        this->title->setColour(c);
        this->artist->setColour(c);
        this->album->setColour(c);
        this->length->setColour(c);
    }

    void ListSong::setW(int w) {
        Element::setW(w);
        this->positionItems();
    }

    void ListSong::positionItems() {
        this->title->setX(this->x() + 15);
        this->artist->setX(this->x() + this->w() * 0.45);
        this->album->setX(this->x() + this->w() * 0.68);
        this->length->setX(this->x() + this->w() - 15 - this->length->w());
        this->title->setY(this->y() + (this->h() - this->title->h())/2);
        this->artist->setY(this->y() + (this->h() - this->artist->h())/2);
        this->album->setY(this->y() + (this->h() - this->album->h())/2);
        this->length->setY(this->y() + (this->h() - this->length->h())/2);

        if (this->title->x() + this->title->w() > this->artist->x() - TEXT_GAP) {
            this->title->setW(this->artist->x() - this->title->x() - TEXT_GAP);
        }
        if (this->artist->x() + this->artist->w() > this->album->x() - TEXT_GAP) {
            this->artist->setW(this->album->x() - this->artist->x() - TEXT_GAP);
        }
        if (this->album->x() + this->album->w() > this->length->x() - TEXT_GAP) {
            this->album->setW(this->length->x() - this->album->x() - TEXT_GAP);
        }
    }
}