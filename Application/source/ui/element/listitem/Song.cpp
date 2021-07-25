#include "ui/element/listitem/Song.hpp"

// Font size of all text
#define FONT_SIZE 22
// Height of item
#define HEIGHT 60
// Pixel gap between text fields
#define TEXT_GAP 30

namespace CustomElm::ListItem {
    Song::Song() : More(HEIGHT) {
        this->title = new Aether::Text(this->x(), this->y(), "", FONT_SIZE, Aether::Render::Wait);
        this->addElement(this->title);
        this->addTexture(this->title);
        this->artist = new Aether::Text(this->x(), this->y(), "", FONT_SIZE, Aether::Render::Wait);
        this->addElement(this->artist);
        this->addTexture(this->artist);
        this->album = new Aether::Text(this->x(), this->y(), "", FONT_SIZE, Aether::Render::Wait);
        this->addElement(this->album);
        this->addTexture(this->album);
        this->length = new Aether::Text(this->x(), this->y(), "", FONT_SIZE, Aether::Render::Wait);
        this->addElement(this->length);
        this->addTexture(this->length);
    }

    void Song::positionElements() {
        this->title->setX(this->x() + 15);
        this->artist->setX(this->x() + this->w() * 0.44);
        this->album->setX(this->x() + this->w() * 0.67);
        this->length->setX(this->x() + this->w() - 50 - this->length->w());
        this->more->setXY(this->x() + this->w() - 25, this->y() + (this->h() - this->more->h())/2 + 1);
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

    void Song::setTitleString(std::string s) {
        this->processText(this->title, [s]() -> Aether::Text * {
            return new Aether::Text(0, 0, s, FONT_SIZE, Aether::Render::Wait);
        });
    }

    void Song::setArtistString(std::string s) {
        this->processText(this->artist, [s]() -> Aether::Text * {
            return new Aether::Text(0, 0, s, FONT_SIZE, Aether::Render::Wait);
        });
    }

    void Song::setAlbumString(std::string s) {
        this->processText(this->album, [s]() -> Aether::Text * {
            return new Aether::Text(0, 0, s, FONT_SIZE, Aether::Render::Wait);
        });
    }

    void Song::setLengthString(std::string s) {
        this->processText(this->length, [s]() -> Aether::Text * {
            return new Aether::Text(0, 0, s, FONT_SIZE, Aether::Render::Wait);
        });
    }

    void Song::setTextColour(Aether::Colour c) {
        this->title->setColour(c);
        this->artist->setColour(c);
        this->album->setColour(c);
        this->length->setColour(c);
    }
}
