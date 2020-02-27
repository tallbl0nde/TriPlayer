#include <algorithm>
#include "Element.hpp"
#include <limits>

// Border size of highlight
#define HIGHLIGHT_SIZE 6

namespace Aether {
    Colour Element::hiBG = Colour{255, 255, 255, 255};
    Colour Element::hiBorder = Colour{255, 255, 255, 255};
    Colour Element::hiSel = Colour{255, 255, 255, 255};
    unsigned int Element::hiSize = HIGHLIGHT_SIZE;
    bool Element::isTouch = false;

    Element::Element(int x, int y, int w, int h) {
        this->setXYWH(x, y, w, h);

        this->parent = nullptr;
        this->hidden_ = false;
        this->callback_ = nullptr;
        this->hasSelectable_ = false;
        this->selectable_ = false;
        this->hasHighlighted_ = false;
        this->highlighted_ = false;
        this->selected_ = false;
        this->hasSelected_ = false;
        this->touchable_ = false;

        this->focussed_ = nullptr;
    }

    int Element::x() {
        return this->x_;
    }

    int Element::y() {
        return this->y_;
    }

    int Element::w() {
        return this->w_;
    }

    int Element::h() {
        return this->h_;
    }

    void Element::setX(int x) {
        int diff = x - this->x();
        this->x_ = x;
        for (size_t i = 0; i < this->children.size(); i++) {
            this->children[i]->setX(this->children[i]->x() + diff);
        }
    }

    void Element::setY(int y) {
        int diff = y - this->y();
        this->y_ = y;
        for (size_t i = 0; i < this->children.size(); i++) {
            this->children[i]->setY(this->children[i]->y() + diff);
        }
    }

    void Element::setW(int w) {
        this->w_ = w;
    }

    void Element::setH(int h) {
        this->h_ = h;
    }

    void Element::setXY(int x, int y) {
        this->setX(x);
        this->setY(y);
    }

    void Element::setWH(int w, int h) {
        this->setW(w);
        this->setH(h);
    }

    void Element::setXYWH(int x, int y, int w, int h) {
        this->setXY(x, y);
        this->setWH(w, h);
    }

    void Element::setParent(Element * p) {
        this->parent = p;
    }

    void Element::addElement(Element * e) {
        e->setParent(this);
        if (e->selectable() || e->hasSelectable()) {
            this->setHasSelectable(true);
        }
        if (e->highlighted() || e->hasHighlighted()) {
            this->setHasHighlighted(true);
        }
        this->children.push_back(e);
    }

    bool Element::removeElement(Element * e) {
        std::vector<Element *>::iterator it = std::find(this->children.begin(), this->children.end(), e);
        if (it != this->children.end()) {
            delete (*it);
            this->children.erase(it);
            return true;
        }
        return false;
    }

    void Element::removeAllElements() {
        while (this->children.size() > 0) {
            delete this->children[0];
            this->children.erase(this->children.begin());
        }
    }

    bool Element::isVisible() {
        if (this->hidden_ || this->x() > 1280 || this->x() + this->w() < 0 || this->y() > 720 || this->y() + this->h() < 0) {
            return false;
        }

        return true;
    }

    bool Element::hidden() {
        return this->hidden_;
    }

    void Element::setHidden(bool b) {
        this->hidden_ = b;
    }

    bool Element::selected() {
        return this->selected_;
    }

    void Element::setSelected(bool b) {
        this->selected_ = b;
        if (this->parent != nullptr) {
            this->parent->setHasSelected(b);
        }
    }

    bool Element::selectable() {
        return this->selectable_;
    }

    void Element::setSelectable(bool b) {
        this->selectable_ = b;
        this->setHasSelectable(b);
    }

    bool Element::touchable() {
        return this->touchable_;
    }

    void Element::setTouchable(bool b) {
        this->touchable_ = b;
    }

    bool Element::highlighted() {
        return this->highlighted_;
    }

    void Element::setHighlighted(bool b) {
        this->highlighted_ = b;
        if (this->parent != nullptr) {
            this->parent->setHasHighlighted(b);
        }
        if (!b) {
            this->setSelected(false);
        }
    }

    std::function<void()> Element::callback() {
        return this->callback_;
    }

    void Element::setCallback(std::function<void()> f) {
        this->callback_ = f;
        this->setSelectable(true);
        this->setTouchable(true);
    }

    bool Element::handleEvent(InputEvent * e) {
        // Handles selecting by either touch or A
        switch (e->type()) {
            case EventType::ButtonPressed:
                if (e->button() == Button::A && this->highlighted_) {
                    this->setSelected(true);
                    return true;
                }
                break;

            case EventType::ButtonReleased:
                if (e->button() == Button::A && this->selected_) {
                    this->setSelected(false);
                    if (this->callback_ != nullptr) {
                        this->callback_();
                        return true;
                    }
                }
                break;

            case EventType::TouchPressed:
                if (e->touchX() >= this->x() && e->touchY() >= this->y() && e->touchX() <= this->x() + this->w() && e->touchY() <= this->y() + this->h() && this->touchable_) {
                    this->setSelected(true);
                    return true;
                }
                break;

            case EventType::TouchMoved:
                if ((e->touchX() < this->x() || e->touchY() < this->y() || e->touchX() > this->x() + this->w() || e->touchY() > this->y() + this->h()) && this->selected_) {
                    if (e->touchX() - e->touchDX() >= this->x() && e->touchY() - e->touchDY() >= this->y() && e->touchX() - e->touchDX() <= this->x() + this->w() && e->touchY() - e->touchDY() <= this->y() + this->h()) {
                        this->setSelected(false);
                        return true;
                    }
                }
                break;

            case EventType::TouchReleased:
                if (e->touchX() >= this->x() && e->touchY() >= this->y() && e->touchX() <= this->x() + this->w() && e->touchY() <= this->y() + this->h() && this->selected_) {
                    this->setSelected(false);
                    if (this->selectable_) {
                        moveHighlight(this);
                    }
                    if (this->callback_ != nullptr) {
                        this->callback_();
                    }
                    return true;
                }
                break;
        }

        return false;
    }

    void Element::update(uint32_t dt) {
        // Do nothing if hidden or off-screen
        if (!this->isVisible()) {
            return;
        }

        // Update children
        for (size_t i = 0; i < this->children.size(); i++) {
            this->children[i]->update(dt);
        }
    }

    bool Element::hasHighlighted() {
        return this->hasHighlighted_;
    }

    void Element::setHasHighlighted(bool b) {
        this->hasHighlighted_ = b;
        if (this->parent != nullptr) {
            this->parent->setHasHighlighted(b);
        }
    }

    bool Element::hasSelectable() {
        return this->hasSelectable_;
    }

    void Element::setHasSelectable(bool b) {
        this->hasSelectable_ = b;
        if (this->parent != nullptr) {
            this->parent->setHasSelectable(b);
        }
    }

    bool Element::hasSelected() {
        return this->hasSelected_;
    }

    void Element::setHasSelected(bool b) {
        this->hasSelected_ = b;
        if (this->parent != nullptr) {
            this->parent->setHasSelected(b);
        }
    }

    void Element::render() {
        // Do nothing if hidden or off-screen
        if (!this->isVisible()) {
            return;
        }

        SDL_BlendMode bld = SDLHelper::getBlendMode();
        if (this->highlighted() && !this->isTouch) {
            SDLHelper::setBlendMode(SDL_BLENDMODE_BLEND);
            this->renderHighlighted();
        }

        if (this->selected()) {
            SDLHelper::setBlendMode(SDL_BLENDMODE_BLEND);
        }

        // Draw children
        for (size_t i = 0; i < this->children.size(); i++) {
            this->children[i]->render();
        }
        SDLHelper::setBlendMode(bld);

        if (this->selected()) {
            // SDLHelper::setBlendMode(SDL_BLENDMODE_BLEND);
            this->renderSelected();
        }
    }

    void Element::renderHighlighted() {
        // Draw background
        SDLHelper::drawFilledRect(this->hiBG, this->x(), this->y(), this->w(), this->h());

        // Draw outline
        SDLHelper::drawRect(this->hiBorder, this->x() - this->hiSize, this->y() - this->hiSize, this->w() + 2*this->hiSize, this->h() + 2*this->hiSize, this->hiSize);
    }

    void Element::renderSelected() {
        SDLHelper::drawFilledRect(this->hiSel, this->x(), this->y(), this->w(), this->h());
    }

    void Element::setActive() {
        this->setHighlighted(true);
    }

    void Element::setInactive() {
        this->setHighlighted(false);
    }

    void Element::setFocussed(Element * e) {
        if (this->focussed_ != nullptr) {
            this->focussed_->setInactive();
        }
        this->focussed_ = e;

        if (e != nullptr) {
            e->setActive();
        }
    }

    Element * Element::focussed() {
        return this->focussed_;
    }

    Element::~Element() {
        if (this->parent != nullptr) {
            if (this->parent->focussed() == this) {
                this->parent->setFocussed(nullptr);
            }
        }
        this->removeAllElements();
    }

    void moveHighlight(Element * e) {
        Element * next = e;

        // First get root element (screen)
        while (e->parent->parent != nullptr) {
            e = e->parent;
        }

        // Set inactive
        e->setInactive();

        // Set element active
        next->setActive();
        next->parent->setFocussed(next);

        // Set focussed up the tree
        e = next;
        while (e->parent->parent != nullptr) {
            e->parent->setFocussed(e);
            e = e->parent;
        }
    }
};