#include "ui/element/listitem/Playlist.hpp"

// Font sizes
#define NAME_FONT_SIZE 28
#define SONGS_FONT_SIZE 20
// Height of item
#define HEIGHT 100
// Padding around image
#define PADDING 12

namespace CustomElm::ListItem {
    Playlist::Playlist(const std::string & img, const bool showMore) : More(HEIGHT) {
        this->image = new Aether::Image(this->x() + PADDING, this->y() + PADDING, img, Aether::Render::Wait);
        this->addElement(this->image);
        this->addTexture(this->image);
        this->name = new Aether::Text(this->x(), this->y(), "", NAME_FONT_SIZE, Aether::Render::Wait);
        this->name->setCanScroll(false);
        this->name->setScrollPause(1000);
        this->name->setScrollSpeed(50);
        this->addElement(this->name);
        this->addTexture(this->name);
        this->songs = new Aether::Text(this->x(), this->y(), "", SONGS_FONT_SIZE, Aether::Render::Wait);
        this->addElement(this->songs);
        this->addTexture(this->songs);
        this->showMore = showMore;
    }

    void Playlist::update(uint32_t dt) {
        More::update(dt);

        if (this->highlighted() && !this->name->canScroll()) {
            this->name->setCanScroll(true);

        } else if (!this->highlighted() && this->name->canScroll()) {
            this->name->setCanScroll(false);
        }

        if (!this->showMore) {
            this->more->setHidden(true);
        }
    }

    void Playlist::positionElements() {
        this->image->setWH(HEIGHT - 2*PADDING, HEIGHT - 2*PADDING);

        this->name->setX(this->image->x() + this->image->w() + 2*PADDING);
        this->name->setY(this->y() + 0.38*HEIGHT - this->name->h()/2);
        int maxW = (this->x() + this->w()) - this->name->x() - (3*PADDING) - this->more->w();
        if (this->name->textureWidth() > maxW) {
            this->name->setW(maxW);
        } else {
            this->name->setW(this->name->textureWidth());
        }

        this->songs->setX(this->name->x());
        this->songs->setY(this->y() + 0.55*HEIGHT);
        this->more->setXY(this->x() + this->w() - 25, this->y() + (this->h() - this->more->h())/2 + 1);
    }

    void Playlist::setNameString(const std::string & s) {
        this->processText(this->name, [s]() -> Aether::Text * {
            return new Aether::Text(0, 0, s, NAME_FONT_SIZE, Aether::Render::Wait);
        });
    }

    void Playlist::setSongsString(const std::string & s) {
        this->processText(this->songs, [s]() -> Aether::Text * {
            return new Aether::Text(0, 0, s, SONGS_FONT_SIZE, Aether::Render::Wait);
        });
    }

    void Playlist::setMutedTextColour(Aether::Colour c) {
        this->songs->setColour(c);
    }

    void Playlist::setTextColour(Aether::Colour c) {
        this->name->setColour(c);
    }
}
