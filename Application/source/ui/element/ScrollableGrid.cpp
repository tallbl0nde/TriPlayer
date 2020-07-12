#include "ui/element/ScrollableGrid.hpp"

// Variables to alter scroll animation
#define CATCHUP 13.5
#define DAMPENING 20
#define MAX_VELOCITY 70

// Padding for very bottom elements
#define PADDING 40
// Padding either side of items (to allow for scroll bar)
#define SIDE_PADDING 50
// Amount touch can deviate (in px) before scrolling
#define TOUCH_RADIUS 30

// Minimum height of scroll bar
#define MIN_SCROLLBAR_SIZE 100
// Width of scroll bar
#define SCROLLBAR_WIDTH 5

namespace CustomElm {
    ScrollableGrid::ScrollableGrid(int x, int y, int w, int h, unsigned int rh, unsigned int c) : Container(x, y, w, h) {
        this->cols = c;
        this->isScrolling = false;
        this->isTouched = false;
        this->rowHeight = rh;
        this->scrollVelocity = 0;
        this->scrollPos = 0;
        this->maxScrollPos = 0;
        this->scrollBar = nullptr;
        this->scrollBarColour = Aether::Colour{255, 255, 255, 255};
        this->showScrollBar_ = true;
        this->touchY = std::numeric_limits<int>::min();
    }

    void ScrollableGrid::positionChild(Aether::Element * e, size_t idx) {
        size_t r = idx/this->cols;
        size_t c = idx%this->cols;

        // Align to center of column horizontally
        size_t m = this->x() + ((c + 0.5)*((this->w() - 2*SIDE_PADDING)/this->cols));
        e->setX(SIDE_PADDING + m - e->w()/2);

        // Align to top of row vertically
        e->setY(this->y() + 10 + r*this->rowHeight - this->scrollPos);
    }

    void ScrollableGrid::stopScrolling() {
        // Move highlight to top element (in same column) if not visible
        if (this->isScrolling) {
            if (this->hasSelectable() && this->focused() != nullptr) {
                if (!(this->focused()->y() >= this->y() && this->focused()->y() + this->focused()->h() <= this->y() + this->h())) {
                    // Find previously selected child and get it's column
                    std::vector<Aether::Element *>::iterator it = std::find(this->children.begin(), this->children.end(), this->focused());
                    if (it != this->children.end()) {
                        size_t col = std::distance(this->children.begin(), it) % cols;

                        // Highlight child in same column
                        for (size_t i = col; i < this->children.size(); i += cols) {
                            if (this->children[i]->y() > this->y() && this->children[i]->selectable()) {
                                this->setFocused(this->children[i]);
                                if (this->parent()->focused() == this) {
                                    this->focused()->setActive();
                                }
                                break;
                            }
                        }
                    }
                }
            }

            this->scrollVelocity = 0;
            this->isScrolling = false;
        }
    }

    void ScrollableGrid::setScrollPos(int pos) {
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
                this->children[i]->setY(this->children[i]->y() - (this->scrollPos - old));
            }
        }
    }

    void ScrollableGrid::updateMaxScrollPos() {
        // The maximum scroll position is simple
        size_t rows = this->children.size()/this->cols + (this->children.size() % this->cols > 0 ? 1 : 0);
        this->maxScrollPos = 2 * PADDING + (rows * rowHeight) - this->h();

        // If children don't take up enough space don't scroll!
        if (this->maxScrollPos <= this->h()) {
            this->maxScrollPos = 0;
            return;
        }

        // Delete scroll bar due to new height
        SDLHelper::destroyTexture(this->scrollBar);
        this->scrollBar = nullptr;
    }

    void ScrollableGrid::addElement(Aether::Element * e) {
        Container::addElementAt(e, this->children.size());
        this->positionChild(e, this->children.size() - 1);
        this->updateMaxScrollPos();
    }

    void ScrollableGrid::removeAllElements() {
        this->cols = 0;
        this->stopScrolling();
        Container::removeAllElements();
        this->setScrollPos(0);
        this->updateMaxScrollPos();
    }

    bool ScrollableGrid::handleEvent(Aether::InputEvent * e) {
        if (e->type() == Aether::EventType::TouchPressed) {
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

                this->touchY = e->touchY();
                if (this->isScrolling) {
                    this->scrollVelocity = 0;
                } else {
                    // If not scrolling pass event (ie. select)
                    Container::handleEvent(e);
                }
                return true;
            }

        } else if (e->type() == Aether::EventType::TouchMoved) {
            if (this->isTouched) {
                // Check touchY and change from tap to swipe if outside threshold
                if (this->touchY != std::numeric_limits<int>::min()) {
                    if (e->touchY() > this->touchY + TOUCH_RADIUS || e->touchY() < this->touchY - TOUCH_RADIUS || e->touchX() < this->x() || e->touchX() > this->x() + this->w()) {
                        for (size_t i = 0; i < this->children.size(); i++) {
                            if (this->children[i]->selected() || this->children[i]->hasSelected()) {
                                this->children[i]->setInactive();
                                break;
                            }
                        }
                        this->touchY = std::numeric_limits<int>::min();
                    }
                } else {
                    this->setScrollPos(this->scrollPos - e->touchDY());
                    this->scrollVelocity = -e->touchDY();
                    if (this->scrollVelocity > MAX_VELOCITY) {
                        this->scrollVelocity = MAX_VELOCITY;
                    } else if (this->scrollVelocity < -MAX_VELOCITY) {
                        this->scrollVelocity = -MAX_VELOCITY;
                    }
                }

                return true;
            }

        } else if (e->type() == Aether::EventType::TouchReleased) {
            if (this->isTouched) {
                this->isTouched = false;
                this->touchY = std::numeric_limits<int>::min();
                Container::handleEvent(e);
                this->isScrolling = true;
                return true;
            }
        }

        // Not a touch event, so let the container handle it
        this->stopScrolling();
        return Container::handleEvent(e);
    }


    void ScrollableGrid::update(uint32_t dt) {
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

        // Recreate scroll bar texture if needed
        if (this->scrollBar == nullptr) {
            int size = (0.8*this->h()) * (this->h()/(double)(this->h() + this->maxScrollPos/3));
            if (size < MIN_SCROLLBAR_SIZE) {
                size = MIN_SCROLLBAR_SIZE;
            }
            this->scrollBar = SDLHelper::renderFilledRoundRect(SCROLLBAR_WIDTH, size, SCROLLBAR_WIDTH/2);
        }

        // If focused element is not completely inside list scroll to it
        if (!this->isScrolling && !this->isTouched && !this->isTouch && this->maxScrollPos != 0 && this->focused() != nullptr) {
            // Check if above
            if (this->focused()->y() < this->y() + PADDING) {
                this->setScrollPos(this->scrollPos + (CATCHUP * (this->focused()->y() - (this->y() + PADDING)) * (dt/1000.0)));

            // And below ;)
            } else if (this->focused()->y() + this->focused()->h() > this->y() + this->h() - (PADDING*2)) {
                this->setScrollPos(this->scrollPos - (CATCHUP * ((this->y() + this->h() - (PADDING*2)) - (this->focused()->y() + this->focused()->h())) * (dt/1000.0)));
            }
        }
    }

    void ScrollableGrid::render() {
        // Set clip rectangle to match scrollable position + size
        SDLHelper::setClip(this->x(), this->y(), this->x() + this->w(), this->y() + this->h());
        Container::render();
        SDLHelper::resetClip();

        // Draw scroll bar
        if (this->maxScrollPos != 0 && this->showScrollBar_ && this->scrollBar != nullptr) {
            int w, h;
            SDLHelper::getDimensions(this->scrollBar, &w, &h);
            int yPos = this->y() + PADDING/2 + (((float)this->scrollPos / this->maxScrollPos) * (this->h() - h - PADDING));
            SDLHelper::drawTexture(this->scrollBar, this->scrollBarColour, this->x() + this->w() - w, yPos);
        }
    }

    void ScrollableGrid::setW(int w) {
        Container::setW(w);

        // Position children again
        for (size_t i = 0; i < this->children.size(); i++) {
            this->positionChild(this->children[i], i);
        }
    }

    void ScrollableGrid::setH(int h) {
        Container::setH(h);
        this->updateMaxScrollPos();
    }

    void ScrollableGrid::setShowScrollBar(bool b) {
        this->showScrollBar_ = b;
    }

    void ScrollableGrid::setScrollBarColour(Aether::Colour c) {
        this->scrollBarColour = c;
    }

    ScrollableGrid::~ScrollableGrid() {
        SDLHelper::destroyTexture(this->scrollBar);
    }
};