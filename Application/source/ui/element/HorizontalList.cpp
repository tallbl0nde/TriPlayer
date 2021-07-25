#include "ui/element/HorizontalList.hpp"

// Variables to alter scroll animation
#define CATCHUP 13.5
#define DAMPENING 20
#define MAX_VELOCITY 70

// Padding on sides
#define X_PADDING 40
#define Y_PADDING 10

// Amount touch can deviate(in px) before scrolling
#define TOUCH_RADIUS_X 50
#define TOUCH_RADIUS_Y 30

namespace CustomElm {
    HorizontalList::HorizontalList(int x, int y, int w, int h) : Aether::Container(x, y, w, h) {
        this->isScrolling = false;
        this->isTouched = false;
        this->scrollVelocity = 0;
        this->scrollPos = 0;
        this->maxScrollPos = 0;
        this->touchX = std::numeric_limits<int>::min();
        this->touchY = std::numeric_limits<int>::min();
    }

    void HorizontalList::setScrollPos(int pos) {
        int old = this->scrollPos;
        if (pos < 0) {
            this->scrollPos = 0;
        } else if (pos > this->maxScrollPos) {
            this->scrollPos = this->maxScrollPos;
        } else {
            this->scrollPos = pos;
        }

        // Update children positions
        if (old != this->scrollPos) {
            for (size_t i = 0; i < this->children.size(); i++) {
                this->children[i]->setX(this->children[i]->x() - (this->scrollPos - old));
            }
        }
    }

    void HorizontalList::stopScrolling() {
        // Move highlight to left element if not visible
        if (this->isScrolling) {
            if (this->hasSelectable() && this->focused() != nullptr) {
                if (!(this->focused()->x() >= this->x() && this->focused()->x() + this->focused()->w() <= this->x() + this->w())) {
                    for (size_t i = 0; i < this->children.size(); i++) {
                        if (this->children[i]->x() > this->x() && this->children[i]->selectable()) {
                            this->setFocused(this->children[i]);
                            if (this->parent()->focused() == this) {
                                this->focused()->setActive();
                            }
                            break;
                        }
                    }
                }
            }

            this->scrollVelocity = 0;
            this->isScrolling = false;
        }
    }

    void HorizontalList::updateMaxScrollPos() {
        this->maxScrollPos = 0;
        if (this->children.size() == 0) {
            return;
        }
        this->maxScrollPos = 2*X_PADDING;

        // Loop over child elements and determine maximum x pos
        for (size_t i = 0; i < this->children.size(); i++) {
            this->maxScrollPos += this->children[i]->w();
        }

        // If children don't take up enough space don't scroll
        if (this->maxScrollPos <= this->w()) {
            this->maxScrollPos = 0;
            return;
        }

        // Subtract this element's width as it wasn't accounted for earlier
        this->maxScrollPos -= this->w();
    }

    void HorizontalList::addElement(Aether::Element * e) {
        // Position element first
        e->setY(this->y() + Y_PADDING);
        if (this->children.empty()) {
            e->setX(this->x() + X_PADDING);
        } else {
            Aether::Element * last = this->children[this->children.size() - 1];
            e->setX(last->x() + last->w());
        }

        // Now add as child
        Aether::Container::addElement(e);
        this->updateMaxScrollPos();
    }

    void HorizontalList::removeAllElements() {
        this->stopScrolling();
        Container::removeAllElements();
        this->setScrollPos(0);
        this->updateMaxScrollPos();
    }

    bool HorizontalList::handleEvent(Aether::InputEvent * e) {
        switch (e->type()) {
            case Aether::EventType::TouchPressed:
                if (e->touchX() >= this->x() && e->touchX() <= this->x() + this->w() && e->touchY() >= this->y() && e->touchY() <= this->y() + this->h()) {
                    // Activate this element
                    this->isTouched = true;

                    // Note we need to traverse up the tree in order to ensure scrollable is focussed
                    Element * elm = this->parent();
                    if (elm != nullptr) {
                        while (elm->parent() != nullptr) {
                            elm->parent()->setFocused(elm);
                            elm = elm->parent();
                        }
                    }

                    // Now set scrollable focussed
                    this->parent()->setFocused(this);

                    this->touchX = e->touchX();
                    this->touchY = e->touchY();
                    if (this->isScrolling) {
                        this->scrollVelocity = 0;
                    } else {
                        // If not scrolling pass event (ie. select)
                        Container::handleEvent(e);
                    }
                    return true;
                }
                break;

            case Aether::EventType::TouchMoved:
                if (this->isTouched) {
                    // Check touch and change from tap to swipe if outside threshold or deactivate
                    if (this->touchX != std::numeric_limits<int>::min() && this->touchY != std::numeric_limits<int>::min()) {
                        if ((e->touchX() > this->touchX + TOUCH_RADIUS_X || e->touchX() < this->touchX - TOUCH_RADIUS_X) ||
                            (e->touchY() > this->touchY + TOUCH_RADIUS_Y || e->touchY() < this->touchY - TOUCH_RADIUS_Y)) {
                            for (size_t i = 0; i < this->children.size(); i++) {
                                if (this->children[i]->selected() || this->children[i]->hasSelected()) {
                                    this->children[i]->setInactive();
                                    break;
                                }
                            }

                            // Deactivate if outside of y threshold
                            if (e->touchY() > this->touchY + TOUCH_RADIUS_Y || e->touchY() < this->touchY - TOUCH_RADIUS_Y) {
                                this->isTouched = false;
                            }

                            this->touchX = std::numeric_limits<int>::min();
                            this->touchY = std::numeric_limits<int>::min();
                        }

                    // If we're set to scroll, scroll!
                    } else {
                        this->setScrollPos(this->scrollPos - e->touchDX());
                        this->scrollVelocity = -e->touchDX();
                        if (this->scrollVelocity > MAX_VELOCITY) {
                            this->scrollVelocity = MAX_VELOCITY;
                        } else if (this->scrollVelocity < -MAX_VELOCITY) {
                            this->scrollVelocity = -MAX_VELOCITY;
                        }
                    }
                    return true;
                }
                break;

            case Aether::EventType::TouchReleased:
                if (this->isTouched) {
                    this->isTouched = false;
                    this->touchX = std::numeric_limits<int>::min();
                    this->touchY = std::numeric_limits<int>::min();
                    Container::handleEvent(e);
                    this->isScrolling = true;
                    return true;
                }
                break;

            // Buttons are handled as a container would
            default:
                this->stopScrolling();
                return Container::handleEvent(e);
                break;
        }

        return false;
    }

    void HorizontalList::update(uint32_t dt) {
        // Update all children first
        Container::update(dt);

        // If scrolling due to touch event
        if (this->isScrolling) {
            this->setScrollPos(this->scrollPos + this->scrollVelocity);
            if (this->scrollPos == 0 || this->scrollPos == this->maxScrollPos) {
                this->scrollVelocity = 0;
            }

            if (this->scrollVelocity < 0) {
                this->scrollVelocity += DAMPENING * (dt/1000.0);
            } else if (this->scrollVelocity > 0) {
                this->scrollVelocity -= DAMPENING * (dt/1000.0);
            }

            if (this->scrollVelocity > -1 && this->scrollVelocity < 1) {
                this->stopScrolling();
            }
        }

        // If focused element is not completely inside list scroll to it
        if (!this->isScrolling && !this->isTouched && !this->isTouch && this->maxScrollPos != 0 && this->focused() != nullptr) {
            // Check if above
            if (this->focused()->x() < this->x() + X_PADDING) {
                this->setScrollPos(this->scrollPos + (CATCHUP * (this->focused()->x() - (this->x() + X_PADDING)) * (dt/1000.0)));

            // And below ;)
            } else if (this->focused()->x() + this->focused()->w() > this->x() + this->w() - (X_PADDING*2)) {
                this->setScrollPos(this->scrollPos - (CATCHUP * ((this->x() + this->w() - (X_PADDING*2)) - (this->focused()->x() + this->focused()->w())) * (dt/1000.0)));
            }
        }
    }

    void HorizontalList::render() {
        // Set clip rectangle to match position + size
        this->renderer->setClipArea(this->x(), this->y(), this->x() + this->w(), this->y() + this->h());

        // Render children
        Container::render();

        // Remove clip rectangle
        this->renderer->resetClipArea();
    }

    void HorizontalList::setW(int w) {
        Container::setW(w);
        this->updateMaxScrollPos();
    }

    void HorizontalList::setH(int h) {
        Container::setH(h);
        for (size_t i = 0; i < this->children.size(); i++) {
            this->children[i]->setH(this->h() - 2*Y_PADDING);
        }
    }
};
