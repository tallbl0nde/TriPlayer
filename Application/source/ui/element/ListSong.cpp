#include "ListSong.hpp"

#define FONT_SIZE 22
#define HEIGHT 60

namespace CustomElm {
    SDL_Texture * ListSong::lineTexture = nullptr;

    ListSong::ListSong(Aether::ThreadQueue * tq) : Element(0, 0, 100, HEIGHT) {
        this->title = new Aether::Exp::ThreadedText(tq, this->x(), this->y(), "", FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->title->setHidden(true);
        this->addElement(this->title);
        this->artist = new Aether::Exp::ThreadedText(tq, this->x(), this->y(), "", FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->artist->setHidden(true);
        this->addElement(this->artist);
        this->album = new Aether::Exp::ThreadedText(tq, this->x(), this->y(), "", FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->album->setHidden(true);
        this->addElement(this->album);
        this->length = new Aether::Exp::ThreadedText(tq, this->x(), this->y(), "", FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->length->setHidden(true);
        this->addElement(this->length);

        // Create line texture if it doesn't exist
        if (this->lineTexture == nullptr) {
            this->lineTexture = SDLHelper::renderFilledRect(this->w(), 1);
        }
        this->lineColour = Aether::Colour{255, 255, 255, 255};
    }

    void ListSong::update(uint32_t dt) {
        Element::update(dt);

        // Once all are rendered position and show!
        if (this->title->hidden()) {
            if (this->title->textureReady() && this->artist->textureReady() && this->album->textureReady() && this->length->textureReady()) {
                this->positionItems();
                this->title->setHidden(false);
                this->artist->setHidden(false);
                this->album->setHidden(false);
                this->length->setHidden(false);
            }
        }
    }

    void ListSong::render() {
        Element::render();
        if (!this->title->hidden()) {
            SDLHelper::drawTexture(this->lineTexture, this->lineColour, this->x(), this->y());
            SDLHelper::drawTexture(this->lineTexture, this->lineColour, this->x(), this->y() + this->h());
        }
    }

    void ListSong::setTitleString(std::string s) {
        this->title->setString(s);
        this->title->startRendering();
    }

    void ListSong::setArtistString(std::string s) {
        this->artist->setString(s);
        this->artist->startRendering();
    }

    void ListSong::setAlbumString(std::string s) {
        this->album->setString(s);
        this->album->startRendering();
    }

    void ListSong::setLengthString(std::string s) {
        this->length->setString(s);
        this->length->startRendering();
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

        if (this->title->x() + this->title->w() > this->artist->x()) {
            this->title->setW(this->artist->x() - this->title->x() - 35);
        }
        if (this->artist->x() + this->artist->w() > this->album->x()) {
            this->artist->setW(this->album->x() - this->artist->x() - 35);
        }
        if (this->album->x() + this->album->w() > this->length->x()) {
            this->album->setW(this->length->x() - this->album->x() - 35);
        }

        // Resize line texture if it is not the right size
        int tw, th;
        SDLHelper::getDimensions(this->lineTexture, &tw, &th);
        if (tw != this->w()) {
            SDLHelper::destroyTexture(this->lineTexture);
            SDLHelper::renderFilledRect(this->w(), 1);
        }
    }
}