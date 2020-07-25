#include "ui/element/ListArtist.hpp"
#include "ui/overlay/ArtistList.hpp"

// Fixed dimensions
#define WIDTH 450
#define HEIGHT 600

namespace CustomOvl {
    ArtistList::ArtistList() {
        // Close if B pressed
        this->onButtonPress(Aether::Button::B, [this]() {
            this->close();
        });

        // Add a second transparent layer cause we like it dark
        Aether::Rectangle * r = new Aether::Rectangle(this->x(), this->y(), this->w(), this->h());
        r->setColour(Aether::Colour{0, 0, 0, 120});
        this->addElement(r);

        // Background rectangle has a fixed size
        this->bg = new Aether::Rectangle(this->x() + (this->w() - WIDTH)/2, this->y() + (this->h() - HEIGHT)/2, WIDTH, HEIGHT, 25);
        this->addElement(this->bg);

        // Create and add empty list (no scrollbar)
        this->list = new Aether::List(this->bg->x() - 25, this->bg->y(), this->bg->w() + 50, this->bg->h());
        this->list->setShowScrollBar(false);
        this->addElement(this->list);
        this->setFocussed(this->list);
        this->touchOutside = false;
    }

    bool ArtistList::handleEvent(Aether::InputEvent * e) {
        if (Overlay::handleEvent(e)) {
            return true;
        }

        // Mark as touched outside
        if (e->type() == Aether::EventType::TouchPressed) {
            if (e->touchX() < this->bg->x() || e->touchX() > this->bg->x() + this->bg->w() || e->touchY() < this->bg->y() || e->touchY() > this->bg->y() + this->bg->h()) {
                this->touchOutside = true;
            }

        // Close overlay if released outside too
        } else if (e->type() == Aether::EventType::TouchReleased) {
            bool b = this->touchOutside;
            this->touchOutside = false;
            if (b && (e->touchX() < this->bg->x() || e->touchX() > this->bg->x() + this->bg->w() || e->touchY() < this->bg->y() || e->touchY() > this->bg->y() + this->bg->h())) {
                this->close();
                return true;
            }
        }

        return false;
    }

    void ArtistList::setBackgroundColour(Aether::Colour c) {
        this->bg->setColour(c);
    }

    void ArtistList::addArtist(CustomElm::ListArtist * l) {
        this->list->addElement(l);
    }
};