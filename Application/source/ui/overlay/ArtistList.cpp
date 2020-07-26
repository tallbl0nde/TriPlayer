#include "ui/element/ListArtist.hpp"
#include "ui/overlay/ArtistList.hpp"

// Fixed dimensions
#define WIDTH 450
#define HEIGHT 600

namespace CustomOvl {
    ArtistList::ArtistList() : Overlay() {
        // Background rectangle has a fixed size
        this->bg = new Aether::Rectangle(this->x() + (this->w() - WIDTH)/2, this->y() + (this->h() - HEIGHT)/2, WIDTH, HEIGHT, 25);
        this->addElement(this->bg);
        this->setTopLeft(this->bg->x(), this->bg->y());
        this->setBottomRight(this->bg->x() + this->bg->w(), this->bg->y() + this->bg->h());

        // Create and add empty list (no scrollbar)
        this->list = new Aether::List(this->bg->x() - 25, this->bg->y(), this->bg->w() + 50, this->bg->h());
        this->list->setShowScrollBar(false);
        this->addElement(this->list);
        this->setFocussed(this->list);
    }

    void ArtistList::setBackgroundColour(Aether::Colour c) {
        this->bg->setColour(c);
    }

    void ArtistList::addArtist(CustomElm::ListArtist * l) {
        this->list->addElement(l);
    }
};