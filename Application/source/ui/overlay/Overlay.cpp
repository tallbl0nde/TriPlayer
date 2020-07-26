#include "ui/overlay/Overlay.hpp"

namespace CustomOvl {
    Overlay::Overlay() : Aether::Overlay() {
        // Close if B pressed
        this->onButtonPress(Aether::Button::B, [this]() {
            this->close();
        });

        // Add a second transparent layer cause we like it dark
        Aether::Rectangle * r = new Aether::Rectangle(this->x(), this->y(), this->w(), this->h());
        r->setColour(Aether::Colour{0, 0, 0, 120});
        this->addElement(r);

        // Mark whole overlay as 'closeable'
        this->setTopLeft(0, 0);
        this->setBottomRight(0, 0);
        this->touched = false;
    }

    void Overlay::setTopLeft(int x, int y) {
        this->x1 = x;
        this->y1 = y;
    }

    void Overlay::setBottomRight(int x, int y) {
        this->x2 = x;
        this->y2 = y;
    }

    bool Overlay::handleEvent(Aether::InputEvent * e) {
        // First allow children to handle the event
        if (Aether::Overlay::handleEvent(e)) {
            return true;
        }

        // Mark as touched outside
        if (e->type() == Aether::EventType::TouchPressed) {
            if (e->touchX() < this->x1 || e->touchX() > this->x2 || e->touchY() < this->y1 || e->touchY() > this->y2) {
                this->touched = true;
            }

        // Close overlay if released outside too
        } else if (e->type() == Aether::EventType::TouchReleased) {
            bool b = this->touched;
            this->touched = false;
            if (b && (e->touchX() < this->x1 || e->touchX() > this->x2 || e->touchY() < this->y1 || e->touchY() > this->y2)) {
                this->close();
                return true;
            }
        }

        return false;
    }
};