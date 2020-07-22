#include "ui/element/ListArtist.hpp"
#include "ui/overlay/menu/ArtistList.hpp"

// Fixed dimensions
#define WIDTH 450
#define HEIGHT 600

namespace CustomOvl::Menu {
    ArtistList::ArtistList() : Menu(Type::Normal) {
        // Background rectangle has a fixed size
        this->bg->setRectSize(WIDTH, HEIGHT);
        this->bg->setXY(this->x() + (this->w() - this->bg->w())/2, this->y() + (this->h() - this->bg->h())/2);

        // Create and add empty list (no scrollbar)
        this->list = new Aether::List(this->bg->x() - 25, this->bg->y(), this->bg->w() + 50, this->bg->h());
        this->list->setShowScrollBar(false);
        this->addElement(this->list);
        this->setFocussed(this->list);
    }

    void ArtistList::addArtist(CustomElm::ListArtist * l) {
        this->list->addElement(l);
    }
};