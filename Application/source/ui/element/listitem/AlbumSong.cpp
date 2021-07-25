#include "ui/element/listitem/AlbumSong.hpp"

// Font size of all text
#define FONT_SIZE 22
// Height of item
#define HEIGHT 60
// Pixel gap between text fields
#define TEXT_GAP 20

namespace CustomElm::ListItem {
    AlbumSong::AlbumSong() : More(HEIGHT) {
        this->track = new Aether::Text(this->x(), this->y(), "", FONT_SIZE, Aether::Render::Wait);
        this->title = new Aether::Text(this->x(), this->y(), "", FONT_SIZE, Aether::Render::Wait);
        this->artist = new Aether::Text(this->x(), this->y(), "", FONT_SIZE, Aether::Render::Wait);
        this->length = new Aether::Text(this->x(), this->y(), "", FONT_SIZE, Aether::Render::Wait);
    }

    void AlbumSong::positionElements() {
        this->track->setX(this->x() + 15);
        this->title->setX(this->x() + this->w() * 0.08);
        this->artist->setX(this->x() + this->w() * 0.6);
        this->length->setX(this->x() + this->w() - 50 - this->length->w());
        this->more->setXY(this->x() + this->w() - 25, this->y() + (this->h() - this->more->h())/2 + 1);
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

    void AlbumSong::setTrackString(std::string s) {
        this->processText(this->track, [s]() -> Aether::Text * {
            return new Aether::Text(0, 0, s, FONT_SIZE, Aether::Render::Wait);
        });
    }

    void AlbumSong::setTitleString(std::string s) {
        this->processText(this->title, [s]() -> Aether::Text * {
            return new Aether::Text(0, 0, s, FONT_SIZE, Aether::Render::Wait);
        });
    }

    void AlbumSong::setArtistString(std::string s) {
        this->processText(this->artist, [s]() -> Aether::Text * {
            return new Aether::Text(0, 0, s, FONT_SIZE, Aether::Render::Wait);
        });
    }

    void AlbumSong::setLengthString(std::string s) {
        this->processText(this->length, [s]() -> Aether::Text * {
            return new Aether::Text(0, 0, s, FONT_SIZE, Aether::Render::Wait);
        });
    }

    void AlbumSong::setTextColour(Aether::Colour c) {
        this->track->setColour(c);
        this->title->setColour(c);
        this->artist->setColour(c);
        this->length->setColour(c);
    }
}
