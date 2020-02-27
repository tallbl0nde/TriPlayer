#include "ListSong.hpp"

#define FONT_SIZE 22
#define HEIGHT 60

namespace CustomElm {
    ListSong::ListSong() : Element(0, 0, 100, HEIGHT) {
        this->title = new Aether::Text(this->x(), this->y(), "", FONT_SIZE);
        this->addElement(this->title);
        this->artist = new Aether::Text(this->x(), this->y(), "", FONT_SIZE);
        this->addElement(this->artist);
        this->album = new Aether::Text(this->x(), this->y(), "", FONT_SIZE);
        this->addElement(this->album);
        this->length = new Aether::Text(this->x(), this->y(), "", FONT_SIZE);
        this->addElement(this->length);
        this->top = new Aether::Rectangle(this->x(), this->y(), this->w(), 1);
        this->addElement(this->top);
        this->bottom = new Aether::Rectangle(this->x(), this->y() + this->h(), this->w(), 1);
        this->addElement(this->bottom);
    }

    void ListSong::setTitleString(std::string s) {
        this->title->setString(s);
        this->title->setY(this->y() + (this->h() - this->title->h())/2);
    }

    void ListSong::setArtistString(std::string s) {
        this->artist->setString(s);
        this->artist->setY(this->y() + (this->h() - this->artist->h())/2);
    }

    void ListSong::setAlbumString(std::string s) {
        this->album->setString(s);
        this->album->setY(this->y() + (this->h() - this->album->h())/2);
    }

    void ListSong::setLengthString(std::string s) {
        this->length->setString(s);
        this->length->setY(this->y() + (this->h() - this->length->h())/2);
    }

    void ListSong::setLineColour(Aether::Colour c) {
        this->top->setColour(c);
        this->bottom->setColour(c);
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

        if (this->title->x() + this->title->w() > this->artist->x()) {
            this->title->setW(this->artist->x() - this->title->x() - 35);
        }
        if (this->artist->x() + this->artist->w() > this->album->x()) {
            this->artist->setW(this->album->x() - this->artist->x() - 35);
        }
        if (this->album->x() + this->album->w() > this->length->x()) {
            this->album->setW(this->length->x() - this->album->x() - 35);
        }

        this->top->setRectSize(this->w(), 1);
        this->bottom->setRectSize(this->w(), 1);
    }
}