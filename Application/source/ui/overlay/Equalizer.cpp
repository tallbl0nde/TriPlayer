#include "ui/overlay/Equalizer.hpp"

// Height of overlay box
#define HEIGHT 500
// Padding between line and slider
#define PADDING 20
// Heading font size
#define TITLE_FONT_SIZE 26
// Index font size
#define INDEX_FONT_SIZE 20

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

        this->ctrl = new Aether::ControlBar();
        this->ctrl->addControl(Aether::Button::A, "OK");
        this->ctrl->addControl(Aether::Button::B, "Cancel");
        this->ctrl->addControl(Aether::Button::X, "Apply");
        this->ctrl->addControl(Aether::Button::Y, "Reset");
        this->addElement(this->ctrl);

        for (size_t i = 0; i < this->sliders.size(); i++) {
            // Create + position slider
            CustomElm::VSlider * slider = new CustomElm::VSlider(this->x() + 65 + (i * 36), this->top->y() + PADDING, 22, this->bottom->y() - this->top->y() - (2*PADDING) - 40, 10);
            slider->setNudge(2.0);
            this->addElement(slider);
            this->sliders[i] = slider;

            // Create + position index text
            Aether::Text * text = new Aether::Text(slider->x() + slider->w()/2, slider->y() + slider->h() + 10, std::to_string(i+1), INDEX_FONT_SIZE);
            text->setX(text->x() - text->w()/2);
            this->addElement(text);
            this->sliderIndexes[i] = text;
        }

        // Call callback on X and Y
        this->onButtonPress(Aether::Button::X, [this]() {
            if (this->applyCallback != nullptr) {
                this->applyCallback();
            }
        });
        this->onButtonPress(Aether::Button::Y, [this]() {
            if (this->resetCallback != nullptr) {
                this->resetCallback();
            }
        });
    }

    void Equalizer::setApplyCallback(std::function<void()> f) {
        this->applyCallback = f;
    }

    void Equalizer::setResetCallback(std::function<void()> f) {
        this->resetCallback = f;
    }

    std::array<float, 32> Equalizer::getValues() {
        std::array<float, 32> values;
        for (size_t i = 0; i < this->sliders.size(); i++) {
            values[i] = this->sliders[i]->value();
            values[i] /= 100.0;
            values[i] += 0.5;
        }
        return values;
    }

    void Equalizer::setValues(const std::array<float, 32> & arr) {
        for (size_t i = 0; i < arr.size(); i++) {
            this->sliders[i]->setValue((arr[i] - 0.5) * 100.0);
        }
    }

    void Equalizer::setApplyLabel(const std::string & s) {
        this->ctrl->updateControl(Aether::Button::X, s);
    }

    void Equalizer::setBackLabel(const std::string & s) {
        this->ctrl->updateControl(Aether::Button::B, s);
    }

    void Equalizer::setResetLabel(const std::string & s) {
        this->ctrl->updateControl(Aether::Button::Y, s);
    }

    void Equalizer::setOKLabel(const std::string & s) {
        this->ctrl->updateControl(Aether::Button::A, s);
    }

    void Equalizer::setBackgroundColour(const Aether::Colour & c) {
        this->rect->setColour(c);
    }

    void Equalizer::setHeadingColour(const Aether::Colour & c) {
        for (size_t i = 0; i < this->sliderIndexes.size(); i++) {
            this->sliderIndexes[i]->setColour(c);
        }
        this->ctrl->setEnabledColour(c);
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
