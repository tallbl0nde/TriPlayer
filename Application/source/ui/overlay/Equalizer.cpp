#include "ui/overlay/Equalizer.hpp"

// Height of overlay box
#define HEIGHT 500
// Padding between line and slider
#define PADDING 40
// Heading font size
#define TITLE_FONT_SIZE 26

namespace CustomOvl {
    Equalizer::Equalizer(const std::string & s) : Overlay() {
        // Create elements
        this->rect = new Aether::Rectangle(this->x(), this->y() + this->h() - HEIGHT, this->w(), HEIGHT);
        this->addElement(this->rect);
        this->top = new Aether::Rectangle(this->x() + 30, this->rect->y() + 72, this->w() - 60, 1);
        this->addElement(this->top);
        this->bottom = new Aether::Rectangle(this->x() + 30, this->rect->y() + this->rect->h() - 72, this->w() - 60, 1);
        this->addElement(this->bottom);
        this->setTopLeft(this->rect->x(), this->rect->y());
        this->setBottomRight(this->rect->x() + this->rect->w(), this->rect->y() + this->rect->h());

        this->title = new Aether::Text(this->x() + 72, this->rect->y() + 40, s, TITLE_FONT_SIZE);
        this->title->setY(this->title->y() - this->title->h()/2);
        this->addElement(this->title);

        this->ctrl = new Aether::Controls();
        this->ctrl->addItem(new Aether::ControlItem(Aether::Button::A, "OK"));
        this->ctrl->addItem(new Aether::ControlItem(Aether::Button::B, "Back"));
        this->ctrl->addItem(new Aether::ControlItem(Aether::Button::X, "Apply"));
        this->labelOK = "OK";
        this->labelBack = "Back";
        this->labelApply = "Apply";
        this->addElement(this->ctrl);

        for (size_t i = 0; i < this->sliders.size(); i++) {
            CustomElm::VSlider * slider = new CustomElm::VSlider(this->x() + 40 + (i * 32), this->top->y() + PADDING, 22, this->bottom->y() - this->top->y() - (2*PADDING), 10);
            slider->setNudge(2.0);
            this->addElement(slider);
            this->sliders[i] = slider;
        }

        // Call callback on X
        this->onButtonPress(Aether::Button::X, [this]() {
            if (this->applyCallback != nullptr) {
                this->applyCallback();
            }
        });
    }

    void Equalizer::setApplyCallback(std::function<void()> f) {
        this->applyCallback = f;
    }

    std::array<float, 32> Equalizer::getValues() {
        std::array<float, 32> values;
        for (size_t i = 0; i < this->sliders.size(); i++) {
            values[i] = this->sliders[i]->value();
        }
        return values;
    }

    void Equalizer::setValues(const std::array<float, 32> & arr) {
        for (size_t i = 0; i < arr.size(); i++) {
            this->sliders[i]->setValue(arr[i] * 50.0);
        }
    }

    void Equalizer::setApplyLabel(const std::string & s) {
        this->labelApply = s;
        this->removeElement(this->ctrl);
        this->ctrl = new Aether::Controls();
        this->ctrl->addItem(new Aether::ControlItem(Aether::Button::A, this->labelOK));
        this->ctrl->addItem(new Aether::ControlItem(Aether::Button::B, this->labelBack));
        this->ctrl->addItem(new Aether::ControlItem(Aether::Button::X, this->labelApply));
        this->addElement(this->ctrl);
    }

    void Equalizer::setBackLabel(const std::string & s) {
        this->labelBack = s;
        this->removeElement(this->ctrl);
        this->ctrl = new Aether::Controls();
        this->ctrl->addItem(new Aether::ControlItem(Aether::Button::A, this->labelOK));
        this->ctrl->addItem(new Aether::ControlItem(Aether::Button::B, this->labelBack));
        this->ctrl->addItem(new Aether::ControlItem(Aether::Button::X, this->labelApply));
        this->addElement(this->ctrl);
    }

    void Equalizer::setOKLabel(const std::string & s) {
        this->labelOK = s;
        this->removeElement(this->ctrl);
        this->ctrl = new Aether::Controls();
        this->ctrl->addItem(new Aether::ControlItem(Aether::Button::A, this->labelOK));
        this->ctrl->addItem(new Aether::ControlItem(Aether::Button::B, this->labelBack));
        this->ctrl->addItem(new Aether::ControlItem(Aether::Button::X, this->labelApply));
        this->addElement(this->ctrl);
    }

    void Equalizer::setBackgroundColour(const Aether::Colour & c) {
        this->rect->setColour(c);
    }

    void Equalizer::setHeadingColour(const Aether::Colour & c) {
        this->title->setColour(c);
    }

    void Equalizer::setLineColour(const Aether::Colour & c) {
        this->top->setColour(c);
        this->bottom->setColour(c);
    }

    void Equalizer::setSliderBackgroundColour(const Aether::Colour & c) {
        for (size_t i = 0; i < this->sliders.size(); i++) {
            this->sliders[i]->setBarBackgroundColour(c);
        }
    }

    void Equalizer::setSliderForegroundColour(const Aether::Colour & c) {
        for (size_t i = 0; i < this->sliders.size(); i++) {
            this->sliders[i]->setBarForegroundColour(c);
        }
    }

    void Equalizer::setSliderKnobColour(const Aether::Colour & c) {
        for (size_t i = 0; i < this->sliders.size(); i++) {
            this->sliders[i]->setKnobColour(c);
        }
    }
};