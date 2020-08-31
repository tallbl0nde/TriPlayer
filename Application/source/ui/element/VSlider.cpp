#include "ui/element/VSlider.hpp"

// Amount of pixels either side of bar to accept as touch
#define LEEWAY 2

namespace CustomElm {
    VSlider::VSlider(int x, int y, int w, int h, int bw) : Aether::Element(x, y, w, h) {
        this->barBg = new Aether::Rectangle(x + (w - bw)/2, y + w/2, bw, h - w, bw/2 - 1);
        this->addElement(this->barBg);
        this->barFg = new Aether::Rectangle(x + (w - bw)/2, y + w/2, bw, h - w, bw/2 - 1);
        this->addElement(this->barFg);
        this->knob = new Aether::Ellipse(x, y, w);
        this->addElement(this->knob);
        this->setNudge(1);
        this->setValue(0.0);
        this->setSelectable(true);
        this->setTouchable(true);
    }

    // Similar to Element::handleEvent but we have our own behaviour
    bool VSlider::handleEvent(Aether::InputEvent * e) {
        bool b = false;

        // If it's a touch event jump to position
        if (e->type() == Aether::EventType::TouchPressed) {
            if (e->touchX() >= (this->x() - LEEWAY) && e->touchY() >= (this->y() - LEEWAY) && e->touchX() <= this->x() + this->w() + LEEWAY && e->touchY() <= this->y() + this->h() + LEEWAY && !this->hidden()) {
                this->setSelected(true);
                float val = 100.0 * ((this->y() + this->h()) - e->touchY())/(float)this->h();
                this->setValue(val);
                b = true;
            }

        } else if (e->type() == Aether::EventType::TouchMoved && this->selected()) {
            float val = 100.0 * ((this->y() + this->h()) - e->touchY())/(float)this->h();
            this->setValue(val);
            b = true;

        // Leave new value on release
        } else if (e->type() == Aether::EventType::TouchReleased && this->selected()) {
            if (e->touchY() < this->y()) {
                this->setValue(100.0);
            } else if (e->touchY() > this->y() + this->h()) {
                this->setValue(0.0);
            }
            this->setSelected(false);
            moveHighlight(this);
            if (this->callback() != nullptr) {
                this->callback()();
            }
            b = true;

        } else if (e->type() == Aether::EventType::ButtonPressed) {
            // Select on A down
            if (e->button() == Aether::Button::A && this->highlighted()) {
                this->setSelected(true);
                b = true;
            } else if (e->button() == Aether::Button::DPAD_UP) {
                this->setValue(this->value() + this->nudge);
                b = true;
            } else if (e->button() == Aether::Button::DPAD_DOWN) {
                this->setValue(this->value() - this->nudge);
                b = true;
            }

        } else if (e->type() == Aether::EventType::ButtonReleased) {
            if (e->button() == Aether::Button::A && this->selected()) {
                this->setSelected(false);
                if (this->callback() != nullptr) {
                    this->callback()();
                }
                b = true;
            }
        }

        // Note: Children don't handle events!
        return b;
    }

    // Need to override this in order to draw highlight on top of children
    void VSlider::render() {
        // Do nothing if hidden or off-screen
        if (!this->isVisible()) {
            return;
        }

        // Render children next
        for (size_t i = 0; i < this->children.size(); i++) {
            this->children[i]->render();
        }

        // Render selected/held layer
        int w, h;
        if (this->selected()) {
            this->renderSelectionTexture();
            SDLHelper::getDimensions(this->selTex, &w, &h);
            SDLHelper::drawTexture(this->selTex, this->selColour, this->knob->x() + (this->knob->xDiameter() - w)/2, this->knob->y() + (this->knob->yDiameter() - h)/2);
        }

        // Finally render highlight border if needed
        if (this->highlighted() && !this->isTouch) {
            this->renderHighlightTextures();
            SDLHelper::getDimensions(this->hiBorderTex, &w, &h);
            SDLHelper::drawTexture(this->hiBorderTex, this->hiBorderColour, this->knob->x() + ((int)this->knob->xDiameter() - w)/2, this->knob->y() + ((int)this->knob->yDiameter() - h)/2);
        }
    }

    float VSlider::value() {
        return this->value_;
    }

    void VSlider::setValue(float val) {
        float old = this->value_;
        this->value_ = (val < 0.0 ? 0.0 : (val > 100.0 ? 100.0 : val));
        if (old != this->value_) {
            int h = this->barFg->texH() * (this->value_/100.0);
            this->barFg->setY(this->barBg->y() + this->barBg->h() - h);
            this->barFg->setH(h);
            this->barFg->setMask(0, this->barFg->texH() - h, this->barFg->texW(), h);
        }
        this->knob->setY(this->barBg->y() + this->barBg->h() - ((this->value_/100.0) * this->barBg->h()) - this->knob->yDiameter()/2);
    }

    void VSlider::setNudge(float n) {
        this->nudge = n;
    }

    void VSlider::setBarBackgroundColour(const Aether::Colour & c) {
        this->barBg->setColour(c);
    }

    void VSlider::setBarForegroundColour(const Aether::Colour & c) {
        this->barFg->setColour(c);
    }

    void VSlider::setKnobColour(const Aether::Colour & c) {
        this->knob->setColour(c);
    }

    SDL_Texture * VSlider::renderHighlightBG() {
        // No background needed
        return nullptr;
    }

    SDL_Texture * VSlider::renderHighlight() {
        return SDLHelper::renderEllipse(this->knob->xDiameter()/2, this->knob->yDiameter()/2, this->hiSize);
    }

    SDL_Texture * VSlider::renderSelection() {
        return SDLHelper::renderFilledEllipse(this->knob->xDiameter()/2, this->knob->yDiameter()/2);
    }
};