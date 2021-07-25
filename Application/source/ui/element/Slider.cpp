#include "ui/element/Slider.hpp"

// Amount of pixels either side of bar to accept as touch
#define LEEWAY 10

namespace CustomElm {
    Slider::Slider(int x, int y, int w, int h, int bh) : Aether::Element(x, y, w, h) {
        this->bar = new Aether::RoundProgressBar(x + h/2, y + (h - bh)/2, w - h, bh);
        this->addElement(this->bar);
        this->knob = new Aether::Ellipse(x, y, h);
        this->setNudge(1);
        this->setValue(0.0);
        this->addElement(this->knob);
        this->setSelectable(true);
        this->setTouchable(true);
    }

    // Similar to Element::handleEvent but we have our own behaviour
    bool Slider::handleEvent(Aether::InputEvent * e) {
        bool b = false;

        // If it's a touch event jump to position
        if (e->type() == Aether::EventType::TouchPressed) {
            if (e->touchX() >= (this->x() - LEEWAY) && e->touchY() >= (this->y() - LEEWAY) && e->touchX() <= this->x() + this->w() + LEEWAY && e->touchY() <= this->y() + this->h() + LEEWAY && !this->hidden()) {
                this->setSelected(true);
                float val = 100.0 * (e->touchX() - this->x())/(float)this->w();
                if (val < 0.0) {
                    val = 0.0;
                } else if (val > 100.0) {
                    val = 100.0;
                }
                this->setValue(val);
                b = true;
            }

        } else if (e->type() == Aether::EventType::TouchMoved && this->selected()) {
            float val = 100.0 * (e->touchX() - this->x())/(float)this->w();
            if (val < 0.0) {
                val = 0.0;
            } else if (val > 100.0) {
                val = 100.0;
            }
            this->setValue(val);
            b = true;

        // Leave new value on release
        } else if (e->type() == Aether::EventType::TouchReleased && this->selected()) {
            if (e->touchX() < this->x()) {
                this->setValue(0.0);
            } else if (e->touchX() > this->x() + this->w()) {
                this->setValue(100.0);
            }
            this->setSelected(false);
            moveHighlight(this);
            if (this->onPressFunc() != nullptr) {
                this->onPressFunc()();
            }
            b = true;

        } else if (e->type() == Aether::EventType::ButtonPressed) {
            // Select on A down
            if (e->button() == Aether::Button::A && this->highlighted()) {
                this->setSelected(true);
                b = true;
            }

            // Otherwise move in direction
            if (this->selected()) {
                if (e->button() == Aether::Button::DPAD_RIGHT) {
                    this->setValue(this->value() + this->nudge);
                    b = true;
                } else if (e->button() == Aether::Button::DPAD_LEFT) {
                    this->setValue(this->value() - this->nudge);
                    b = true;
                }
            }

        } else if (e->type() == Aether::EventType::ButtonReleased) {
            if (e->button() == Aether::Button::A && this->selected()) {
                this->setSelected(false);
                if (this->onPressFunc() != nullptr) {
                    this->onPressFunc()();
                }
                b = true;
            }
        }

        // Note: Children don't handle events!
        return b;
    }

    // Need to override this in order to draw highlight on top of children
    void Slider::render() {
        // Do nothing if hidden or off-screen
        if (!this->isVisible()) {
            return;
        }

        // Render children next
        for (size_t i = 0; i < this->children.size(); i++) {
            this->children[i]->render();
        }

        // Render selected/held layer
        if (this->selected()) {
            this->renderSelectionTexture();
            this->selTex->setColour(this->selColour);
            this->selTex->render(this->knob->x() + (this->knob->xDiameter() - this->selTex->width())/2, this->knob->y() + (this->knob->yDiameter() - this->selTex->height())/2);
        }

        // Finally render highlight border if needed
        if (this->highlighted() && !this->isTouch) {
            this->renderHighlightTextures();
            this->hiBorderTex->setColour(this->hiBorderColour);
            this->hiBorderTex->render(this->knob->x() + ((int)this->knob->xDiameter() - this->hiBorderTex->width())/2, this->knob->y() + ((int)this->knob->yDiameter() - this->hiBorderTex->height())/2);
        }
    }

    float Slider::value() {
        return this->bar->value();
    }

    void Slider::setValue(float val) {
        this->bar->setValue(val);
        this->knob->setX(this->x() + ((this->w() - this->knob->xDiameter()) * (this->bar->value()/100.0)) - 3);
    }

    void Slider::setNudge(float n) {
        this->nudge = n;
    }

    void Slider::setBarBackgroundColour(Aether::Colour c) {
        this->bar->setBackgroundColour(c);
    }

    void Slider::setBarForegroundColour(Aether::Colour c) {
        this->bar->setForegroundColour(c);
    }

    void Slider::setKnobColour(Aether::Colour c) {
        this->knob->setColour(c);
    }

    void Slider::setW(int w) {
        Element::setW(w);
        this->bar->setW(this->w() - this->knob->xDiameter());
        this->bar->setX(this->x() + this->knob->xDiameter()/2);
        this->knob->setX(this->x() + ((this->w() - this->knob->xDiameter()) * (this->bar->value()/100.0)) - 3);
    }

    void Slider::setH(int h) {
        Element::setH(h);
        this->knob->setXDiameter(h);
        this->knob->setYDiameter(h);
        this->bar->setY(this->y() + (this->h() - this->bar->h())/2);
    }

    void Slider::setBarH(int bh) {
        this->bar->setH(bh);
        this->bar->setY(this->y() + (this->h() - this->bar->h())/2);
    }

    Aether::Drawable * Slider::renderHighlightBG() {
        // No background needed
        return new Aether::Drawable();
    }

    Aether::Drawable * Slider::renderHighlight() {
        return this->renderer->renderEllipseTexture(this->knob->xDiameter()/2, this->knob->yDiameter()/2, this->hiSize);
    }

    Aether::Drawable * Slider::renderSelection() {
        return this->renderer->renderFilledEllipseTexture(this->knob->xDiameter()/2, this->knob->yDiameter()/2);
    }
};
