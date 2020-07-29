#include "ui/element/listitem/Playlist.hpp"

// Font sizes
#define NAME_FONT_SIZE 28
#define SONGS_FONT_SIZE 20
// Height of item
#define HEIGHT 100
// Padding around image
#define PADDING 12

namespace CustomElm::ListItem {
    Playlist::Playlist(const std::string & img) : More(HEIGHT) {
        this->image = new Aether::Image(this->x() + PADDING, this->y() + PADDING, img, 1, 1, Aether::RenderType::Deferred);
        this->watchTexture(this->image);
        this->name = new Aether::Text(this->x(), this->y(), "", NAME_FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->watchTexture(this->name);
        this->songs = new Aether::Text(this->x(), this->y(), "", SONGS_FONT_SIZE, Aether::FontStyle::Regular, Aether::RenderType::Deferred);
        this->watchTexture(this->songs);
    }

    void Playlist::positionItems() {
        this->image->setWH(HEIGHT - 2*PADDING, HEIGHT - 2*PADDING);
        this->name->setX(this->image->x() + this->image->w() + 2*PADDING);
        this->name->setY(this->y() + 0.38*HEIGHT - this->name->h()/2);
        this->songs->setX(this->name->x());
        this->songs->setY(this->y() + 0.55*HEIGHT);
        this->more->setXY(this->x() + this->w() - 25, this->y() + (this->h() - this->more->h())/2 + 1);
    }

    void Playlist::setNameString(const std::string & s) {
        this->name->setString(s);
    }

    void Playlist::setSongsString(const std::string & s) {
        this->songs->setString(s);
    }

    void Playlist::setMutedTextColour(Aether::Colour c) {
        this->songs->setColour(c);
    }

    void Playlist::setTextColour(Aether::Colour c) {
        this->name->setColour(c);
    }
}