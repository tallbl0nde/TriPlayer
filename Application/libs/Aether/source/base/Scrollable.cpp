#include "Scrollable.hpp"

// Default catchup amount
#define DEFAULT_CATCHUP 6
// Default dampening amount
#define DEFAULT_DAMPENING 20
// Maximum scrollVelocity
#define MAX_VELOCITY 70
// Padding for very top and very bottom elements
#define PADDING 40
// Padding either side of items (to allow for scroll bar)
#define SIDE_PADDING 50
// Height of scroll bar
#define SCROLLBAR_SIZE 100
// Amount touch can deviate (in px) before scrolling
#define TOUCH_RADIUS 30

namespace Aether {
    SDL_Texture * Scrollable::scrollBar = nullptr;

    Scrollable::Scrollable(int x, int y, int w, int h) : Container(x, y, w, h) {
        this->canScroll_ = true;
        this->isScrolling = false;
        this->isTouched = false;
        this->scrollCatchup = DEFAULT_CATCHUP;
        this->scrollDampening = DEFAULT_DAMPENING;
        this->scrollVelocity = 0;
        this->scrollPos = 0;
        this->maxScrollPos = 0;
        this->showScrollBar_ = true;
        if (this->scrollBar == nullptr) {
            this->scrollBar = SDLHelper::renderFilledRect(5, SCROLLBAR_SIZE);
        }
        this->scrollBarColour = Colour{255, 255, 255, 255};
        this->touchY = std::numeric_limits<int>::min();
        this->renderTex = SDLHelper::createTexture(w, h);
    }

    void Scrollable::setScrollPos(int pos) {
        unsigned int old = this->scrollPos;
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

    void Scrollable::updateMaxScrollPos() {
        this->maxScrollPos = 0;

        if (this->children.size() == 0) {
            return;
        }

        this->maxScrollPos = 2 * PADDING;

        // Loop over child elements and determine maximum y pos
        for (size_t i = 0; i < this->children.size(); i++) {
            this->maxScrollPos += this->children[i]->h();
        }

        // If children don't take up enough space don't scroll!
        if (this->maxScrollPos <= this->h()) {
            this->maxScrollPos = 0;
            return;
        }

        // Subtract this element's height as it wasn't accounted for earlier
        this->maxScrollPos -= this->h();
    }

    void Scrollable::stopScrolling() {
        if (this->isScrolling) {
            // Move highlight to top element if not visible
            if (this->hasSelectable()) {
                if (!(this->focussed()->y() >= this->y() && this->focussed()->y() + this->focussed()->h() <= this->y() + this->h())) {
                    for (size_t i = 0; i < this->children.size(); i++) {
                        if (this->children[i]->y() > this->y() && this->children[i]->selectable()) {
                            this->setFocussed(this->children[i]);
                            if (this->parent->focussed() == this) {
                                this->focussed()->setActive();
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

    void Scrollable::setW(int w) {
        Container::setW(w);
        SDLHelper::destroyTexture(this->renderTex);
        this->renderTex = SDLHelper::createTexture(this->w(), this->h());
        for (size_t i = 0; i < this->children.size(); i++) {
            this->children[i]->setW(this->w() - 2*SIDE_PADDING);
        }
    }

    void Scrollable::setH(int h) {
        Container::setH(h);
        SDLHelper::destroyTexture(this->renderTex);
        this->renderTex = SDLHelper::createTexture(this->w(), this->h());
        this->updateMaxScrollPos();
    }

    int Scrollable::catchup() {
        return this->scrollCatchup;
    }

    void Scrollable::setCatchup(int c) {
        this->scrollCatchup = c;
    }

    float Scrollable::dampening() {
        return this->scrollDampening;
    }

    void Scrollable::setDampening(float d) {
        this->scrollDampening = d;
    }

    bool Scrollable::showScrollBar() {
        return this->showScrollBar_;
    }

    void Scrollable::setShowScrollBar(bool b) {
        this->showScrollBar_ = b;
    }

    void Scrollable::setScrollBarColour(Colour c) {
        this->scrollBarColour = c;
    }

    void Scrollable::setScrollBarColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        this->setScrollBarColour(Colour{r, g, b, a});
    }

    bool Scrollable::canScroll() {
        return this->canScroll_;
    }

    void Scrollable::setCanScroll(bool b) {
        this->canScroll_ = b;
        if (!b) {
            this->stopScrolling();
        }
    }

    void Scrollable::addElement(Element * e) {
        // Position element at correct position
        e->setX(this->x() + SIDE_PADDING);
        if (this->children.size() == 0) {
            e->setY(this->y() + PADDING);
        } else {
            e->setY(this->children[this->children.size() - 1]->y() + this->children[this->children.size() - 1]->h());
        }
        e->setW(this->w() - 2*SIDE_PADDING);

        Container::addElement(e);
        this->updateMaxScrollPos();
    }

    bool Scrollable::removeElement(Element * e) {
        bool b = Container::removeElement(e);
        if (b) {
            this->updateMaxScrollPos();
        }
        return b;
    }

    void Scrollable::removeAllElements() {
        Container::removeAllElements();
        this->setScrollPos(0);
        this->updateMaxScrollPos();
    }

    bool Scrollable::removeFollowingElements(Element * e) {
        std::vector<Element *>::iterator it = std::find(this->children.begin(), this->children.end(), e);
        // If the element was found loop over remaining elements and delete
        if (it != this->children.end()) {
            size_t i = std::distance(this->children.begin(), it);
            i++;
            while (i < this->children.size()) {
                delete this->children[i];
                this->children.erase(this->children.begin() + i);
            }
            this->updateMaxScrollPos();
            if (this->scrollPos > this->maxScrollPos) {
                this->setScrollPos(this->maxScrollPos);
            }
            return true;
        }
        return false;
    }

    bool Scrollable::handleEvent(InputEvent * e) {
        switch (e->type()) {
            case EventType::TouchPressed:
                if (e->touchX() >= this->x() && e->touchX() <= this->x() + this->w() && e->touchY() >= this->y() && e->touchY() <= this->y() + this->h()) {
                    // Activate this element
                    this->isTouched = true;
                    this->parent->setFocussed(this);

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

            case EventType::TouchMoved:
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
                        if (this->canScroll_) {
                            this->setScrollPos(this->scrollPos - e->touchDY());
                            this->scrollVelocity = -e->touchDY();
                            if (this->scrollVelocity > MAX_VELOCITY) {
                                this->scrollVelocity = MAX_VELOCITY;
                            } else if (this->scrollVelocity < -MAX_VELOCITY) {
                                this->scrollVelocity = -MAX_VELOCITY;
                            }
                        }
                    }

                    return true;
                }
                break;

            case EventType::TouchReleased:
                if (this->isTouched) {
                    this->isTouched = false;
                    this->touchY = std::numeric_limits<int>::min();
                    Container::handleEvent(e);
                    if (this->canScroll_) {
                        this->isScrolling = true;
                    }
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

    void Scrollable::update(uint32_t dt) {
        // Update all children first
        Container::update(dt);

        // If scrolling due to touch event
        if (this->isScrolling) {
            this->setScrollPos(this->scrollPos + this->scrollVelocity);
            if (this->scrollPos == 0 || this->scrollPos == this->maxScrollPos) {
                this->scrollVelocity = 0;
            }

            if (this->scrollVelocity < 0) {
                this->scrollVelocity += this->scrollDampening * (dt/1000.0);
            } else if (this->scrollVelocity > 0) {
                this->scrollVelocity -= this->scrollDampening * (dt/1000.0);
            }

            if (this->scrollVelocity > -1 && this->scrollVelocity < 1) {
                this->stopScrolling();
            }
        }
    }

    void Scrollable::render() {
        SDLHelper::renderToTexture(this->renderTex);
        SDL_BlendMode bld = SDLHelper::getBlendMode();
        SDLHelper::setBlendMode(SDL_BLENDMODE_NONE);
        SDLHelper::setOffset(-this->x(), -this->y());

        Container::render();

        // Reset all rendering calls to screen
        SDLHelper::setOffset(0, 0);
        SDLHelper::setBlendMode(bld);
        SDLHelper::renderToScreen();

        // Render texture
        SDLHelper::drawTexture(this->renderTex, Colour{255, 255, 255, 255}, this->x(), this->y(), this->w(), this->h());

        // Draw scroll bar
        if (this->maxScrollPos != 0 && this->showScrollBar_) {
            int yPos = this->y() + PADDING/2 + (((float)this->scrollPos / this->maxScrollPos) * (this->h() - SCROLLBAR_SIZE - PADDING));
            SDLHelper::drawTexture(this->scrollBar, this->scrollBarColour, this->x() + this->w() - 5, yPos);
        }
    }

    Scrollable::~Scrollable() {
        // I should do this but it's static /shrug
        // SDLHelper::destroyTexture(this->scrollBar);
        SDLHelper::destroyTexture(this->renderTex);
    }
};