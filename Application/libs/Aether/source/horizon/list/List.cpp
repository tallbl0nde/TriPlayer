#include "List.hpp"
#include "utils/Utils.hpp"

// Same as Scrollable.hpp
#define PADDING 40

namespace Aether {
    List::List(int x, int y, int w, int h) : Scrollable(x, y, w, h) {
        this->setShowScrollBar(true);
        this->setCatchup(13.5);
        this->heldButton = Button::NO_BUTTON;
        this->scroll = false;
    }

    bool List::handleEvent(InputEvent * e) {
        // Store result of event
        bool res = Scrollable::handleEvent(e);

        // If button event isn't handled...
        if (!res && this->canScroll_) {
            if ((e->button() == Button::DPAD_DOWN && this->scrollPos < this->maxScrollPos) || (e->button() == Button::DPAD_UP && this->scrollPos > 0)) {
                if (e->type() == EventType::ButtonPressed) {
                    this->heldButton = e->button();
                    this->scroll = true;
                    return true;
                }

                if (e->type() == EventType::ButtonReleased && e->button() == this->heldButton && e->id() != FAKE_ID) {
                    this->heldButton = Button::NO_BUTTON;
                    return true;
                }
            } else {
                this->scroll = false;
            }
        }

        return res;
    }

    void List::update(uint32_t dt) {
        Scrollable::update(dt);

        // Allow "manual" scrolling at top and bottom
        if (this->scroll) {
            if (this->heldButton == Button::DPAD_DOWN) {
                this->setScrollPos(this->scrollPos + (500 * (dt/1000.0)));
                return;
            } else if (this->heldButton == Button::DPAD_UP) {
                this->setScrollPos(this->scrollPos - (500 * (dt/1000.0)));
                return;
            }
        }

        // If focussed element is not completely inside list scroll to it
        if (!this->isScrolling && !this->isTouched && !this->isTouch && this->maxScrollPos != 0 && this->focussed() != nullptr) {
            // Check if above
            if (this->focussed()->y() < this->y() + PADDING) {
                this->setScrollPos(this->scrollPos + (this->scrollCatchup * (this->focussed()->y() - (this->y() + PADDING)) * (dt/1000.0)));

            // And below ;)
            } else if (this->focussed()->y() + this->focussed()->h() > this->y() + this->h() - (PADDING*2)) {
                this->setScrollPos(this->scrollPos - (this->scrollCatchup * ((this->y() + this->h() - (PADDING*2)) - (this->focussed()->y() + this->focussed()->h())) * (dt/1000.0)));
            }
        }
    }
};