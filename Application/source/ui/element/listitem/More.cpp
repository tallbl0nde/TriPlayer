#include "ui/element/listitem/More.hpp"

// Number of pixels around image to count as touched
#define PADDING 20

namespace CustomElm::ListItem {
    More::More(int h) : Item(h) {
        this->moreCallback = nullptr;
        this->more = new Aether::Image(0, 0, "romfs:/icons/verticaldots.png", 1, 1, Aether::RenderType::Deferred);
        this->watchTexture(this->more);
        this->touchedMore = false;
    }

    bool More::handleEvent(Aether::InputEvent * e) {
        // Check if X pressed when focussed
        if (this->highlighted() && e->type() == Aether::EventType::ButtonPressed) {
            if (e->button() == Aether::Button::X) {
                if (this->moreCallback != nullptr) {
                    this->moreCallback();
                }
                return true;
            }
        }

        // Check if pressed over image
        if (e->touchX() >= this->more->x() - PADDING && e->touchY() >= this->more->y() - PADDING && e->touchX() <= this->more->x() + this->more->w() + PADDING && e->touchY() <= this->more->y() + this->more->h() + PADDING) {
            if (e->type() == Aether::EventType::TouchPressed) {
                // Do nothing when pressed
                this->touchedMore = true;
                return true;

            } else if (e->type() == Aether::EventType::TouchReleased && this->touchedMore) {
                if (this->moreCallback != nullptr) {
                    this->moreCallback();
                }
                this->touchedMore = false;
                return true;
            }
        }

        // If not handled let children handle it
        return Item::handleEvent(e);
    }

    void More::setInactive() {
        Item::setInactive();
        this->touchedMore = false;
    }

    void More::setMoreCallback(std::function<void()> f) {
        this->moreCallback = f;
    }

    void More::setMoreColour(Aether::Colour c) {
        this->more->setColour(c);
    }
};