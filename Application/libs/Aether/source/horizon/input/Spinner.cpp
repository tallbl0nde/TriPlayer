#include "Spinner.hpp"

// Default size + dimensions
#define ARROW_FONT_SIZE 30
#define ARROW_PADDING 10
#define LABEL_FONT_SIZE 18
#define LABEL_PADDING 30
#define VALUE_FONT_SIZE 40
#define VALUE_PADDING 20
#define HEIGHT 200  // Height without label

namespace Aether {
    Spinner::Spinner(int x, int y, int w) : Container(x, y, w, HEIGHT) {
        // Up arrow
        Element * e = new Element(0, this->y(), ARROW_FONT_SIZE + (2 * ARROW_PADDING), ARROW_FONT_SIZE + ARROW_PADDING);
        e->setX(this->x() + (this->w() - e->w())/2);
        e->setCallback([this](){
            this->incrementVal();
        });
        e->setSelectable(false);
        this->up = new Text(0, this->y(), "\uE147", ARROW_FONT_SIZE, FontType::Extended);
        this->up->setXY(e->x() + (e->w() - this->up->w())/2, e->y() + (e->h() - this->up->h())/2);
        e->addElement(this->up);
        this->addElement(e);

        // Down arrow
        e = new Element(0, this->y() + this->h(), ARROW_FONT_SIZE + (2 * ARROW_PADDING), ARROW_FONT_SIZE + ARROW_PADDING);
        e->setX(this->x() + (this->w() - e->w())/2);
        e->setY(e->y() - e->h());
        e->setCallback([this](){
            this->decrementVal();
        });
        e->setSelectable(false);
        this->down = new Text(0, 0, "\uE148", ARROW_FONT_SIZE, FontType::Extended);
        this->down->setXY(e->x() + (e->w() - this->down->w())/2, e->y() + (e->h() - this->down->h())/2);
        e->addElement(this->down);
        this->addElement(e);

        // Value string
        e = new Element(0, 0, this->w(), VALUE_FONT_SIZE + VALUE_PADDING * 2);
        e->setXY(this->x() + (this->w() - e->w())/2, this->y() + (this->h() - e->h())/2);
        e->setSelectable(true);
        this->str = new Text(0, 0, "0", VALUE_FONT_SIZE);
        this->str->setXY(e->x() + (e->w() - this->str->w())/2, e->y() + (e->h() - this->str->h())/2);
        e->addElement(this->str);
        this->addElement(e);
        this->setFocussed(e);

        // Label string (set hidden)
        this->label_ = new Text(this->x() + this->w()/2, this->y() + this->h() + LABEL_PADDING, "", LABEL_FONT_SIZE);
        this->label_->setHidden(true);
        this->addElement(this->label_);

        this->wrap = false;
        this->padding = 0;
        this->value_ = 0;
        this->amount = 1;
        this->min_ = 0;
        this->max_ = 0;
        this->arrowC = Colour{255, 255, 255, 255};
        this->highlightC = Colour{255, 255, 255, 255};
        this->textC = Colour{255, 255, 255, 255};
    }

    void Spinner::incrementVal() {
        // Wrap around if enabled
        if (this->wrap && (this->value_ == this->max_)) {
            this->value_ = this->min_;
        } else {
            // Increase by set amount
            this->value_ += this->amount;
            if (this->value_ > this->max_) {
                this->value_ = this->max_;
            }
        }

        // Update string
        this->setValue(this->value_);
    }

    void Spinner::decrementVal() {
        // Wrap around if enabled
        if (this->wrap && (this->value_ == this->min_)) {
            this->value_ = this->max_;
        } else {
            // Decrease by set amount
            this->value_ -= this->amount;
            if (this->value_ < this->min_) {
                this->value_ = this->min_;
            }
        }

        // Update string
        this->setValue(this->value_);
    }

    bool Spinner::handleEvent(InputEvent * e) {
        // Inc/dec on dpad press
        if (e->type() == EventType::ButtonPressed) {
            switch (e->button()) {
                case Button::DPAD_UP:
                    this->incrementVal();
                    return true;
                    break;

                case Button::DPAD_DOWN:
                    this->decrementVal();
                    return true;
                    break;

                default:
                    break;
            }
        }

        // Set this element focussed if arrows touched (they're the only children that can handle events)
        if (Container::handleEvent(e)) {
            this->parent->setFocussed(this);
            return true;
        }

        return false;
    }

    void Spinner::update(uint32_t dt) {
        Container::update(dt);

        // Set colours
        if (this->isFocussed && !this->isTouch) {
            this->label_->setColour(this->highlightC);
            this->up->setColour(this->highlightC);
            this->down->setColour(this->highlightC);
            this->str->setColour(this->highlightC);
        } else {
            this->label_->setColour(this->arrowC);
            this->up->setColour(this->arrowC);
            this->down->setColour(this->arrowC);
            this->str->setColour(this->textC);
        }
    }

    void Spinner::setActive() {
        this->isFocussed = true;
        Container::setActive();
    }

    void Spinner::setInactive() {
        this->isFocussed = false;
        Container::setInactive();
    }

    bool Spinner::wrapAround() {
        return this->wrap;
    }

    void Spinner::setWrapAround(bool b) {
        this->wrap = b;
    }

    unsigned int Spinner::digits() {
        return this->padding;
    }

    void Spinner::setDigits(unsigned int p) {
        this->padding = p;
        this->setValue(this->value_);
    }

    void Spinner::setLabel(std::string s) {
        this->label_->setString(s);

        // Remove if empty
        if (s == "") {
            this->label_->setHidden(true);
            this->setH(HEIGHT);
            return;
        }

        // Otherwise show and adjust height
        this->label_->setHidden(false);
        this->label_->setX(this->x() + (this->w() - this->label_->w())/2);
        this->setH(this->h() + LABEL_PADDING + this->label_->h());
    }

    std::string Spinner::label() {
        return this->label_->string();
    }

    int Spinner::changeAmount() {
        return this->amount;
    }

    void Spinner::setChangeAmount(int a) {
        this->amount = a;
    }

    int Spinner::value() {
        return this->value_;
    }

    void Spinner::setValue(int v) {
        this->value_ = v;
        std::string tmp = std::to_string(this->value_);
        while (tmp.length() < this->padding) {
            tmp = "0" + tmp;
        }
        this->str->setString(tmp);
        this->str->setX(this->x() + (this->w() - this->str->w())/2);
    }

    int Spinner::min() {
        return this->min_;
    }

    void Spinner::setMin(int m) {
        this->min_ = m;
        if (this->value_ < m) {
            this->setValue(m);
        }
    }

    int Spinner::max() {
        return this->max_;
    }

    void Spinner::setMax(int m) {
        this->max_ = m;
        if (this->value_ > m) {
            this->setValue(m);
        }
    }

    Colour Spinner::getArrowColour() {
        return this->arrowC;
    }

    void Spinner::setArrowColour(Colour c) {
        this->arrowC = c;
    }

    Colour Spinner::getHighlightColour() {
        return this->highlightC;
    }

    void Spinner::setHighlightColour(Colour c) {
        this->highlightC = c;
    }

    Colour Spinner::getTextColour() {
        return this->textC;
    }

    void Spinner::setTextColour(Colour c) {
        this->textC = c;
    }
}