#include "Aether/utils/Utils.hpp"
#include "ui/element/ButtonPicker.hpp"

// Dimensions
constexpr int width = 120;
constexpr int height = 120;
constexpr unsigned int radius = 12;
constexpr unsigned int border = 3;
constexpr unsigned int fontSize = 40;

namespace CustomElm {
    ButtonPicker::ButtonPicker(int x, int y, Aether::Button b) : Aether::Element(x, y, width, height) {
        this->active = Aether::Colour{255, 255, 255, 255};
        this->inactive = Aether::Colour{255, 255, 255, 255};
        this->rect = new Aether::Box(this->x(), this->y(), this->w(), this->h(), border, radius);
        this->addElement(this->rect);

        this->str = new Aether::Text(0, 0, "", fontSize);
        this->addElement(this->str);
        this->setSelectedButton(b);
        this->setSelectable(true);
        this->setTouchable(true);
    }

    void ButtonPicker::updateText() {
        this->str->setString(Aether::Utils::buttonToCharacter(this->button));
        this->str->setXY(this->x() + (this->w() - this->str->w())/2, this->y() + (this->h() - this->str->h())/2);
    }

    bool ButtonPicker::handleEvent(Aether::InputEvent * e){
        // Ignore ALL touch events
        if (e->type() == Aether::EventType::TouchPressed || e->type() == Aether::EventType::TouchReleased) {
            return false;
        }

        // Don't do anything special if we're not selected
        if (!this->selected()) {
            return Element::handleEvent(e);
        }

        // Otherwise if we get a button press, capture it and deselect
        if (e->type() == Aether::EventType::ButtonPressed) {
            this->button = e->button();
            this->updateText();
            this->setSelected(false);
            return true;
        }

        // Otherwise don't handle
        return false;
    }

    void ButtonPicker::setSelected(bool b) {
        Element::setSelected(b);
        if (b) {
            // Show ? when going to change
            this->str->setString("?");
            this->str->setXY(this->x() + (this->w() - this->str->w())/2, this->y() + (this->h() - this->str->h())/2);
            this->str->setColour(this->active);
            this->rect->setColour(this->active);

        } else {
            if (this->str->string() == "?") {
                this->str->setString("");
            }
            this->rect->setColour(this->inactive);
            this->str->setColour(this->inactive);
        }
    }

    Aether::Drawable * ButtonPicker::renderHighlight() {
        // Only render if not selected
        if (this->selected()) {
            return new Aether::Drawable();
        }
        return this->renderer->renderRoundRectTexture(this->w() + 4, this->h() + 4, radius, this->hiSize);
    }

    Aether::Drawable * ButtonPicker::renderSelection() {
        // Hide selection
        return new Aether::Drawable();
    }

    Aether::Button ButtonPicker::selectedButton() {
        return this->button;
    }

    void ButtonPicker::setSelectedButton(const Aether::Button b) {
        this->button = b;
        this->updateText();
    }

    void ButtonPicker::setActiveColour(const Aether::Colour & c) {
        this->active = c;
        this->setSelected(this->selected());
    }

    void ButtonPicker::setInactiveColour(const Aether::Colour & c) {
        this->inactive = c;
        this->setSelected(this->selected());
    }
};
