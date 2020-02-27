#include "DateTime.hpp"

// Defaults
#define HEIGHT 470
#define SEP_FONT_SIZE 32
#define TITLE_FONT_SIZE 26

namespace Aether {
    DateTime::DateTime(std::string s, struct tm & t, DTFlag d) : Overlay(), refTm(t) {
        // Create elements
        this->rect = new Rectangle(this->x(), this->y() + this->h() - HEIGHT, this->w(), HEIGHT);
        this->addElement(this->rect);
        this->top = new Rectangle(this->x() + 30, this->rect->y() + 72, this->w() - 60, 1);
        this->addElement(this->top);
        this->bottom = new Rectangle(this->x() + 30, this->rect->y() + this->rect->h() - 72, this->w() - 60, 1);
        this->addElement(this->bottom);

        this->title = new Text(this->x() + 72, this->rect->y() + 40, s, TITLE_FONT_SIZE);
        this->title->setY(this->title->y() - this->title->h()/2);
        this->addElement(this->title);

        this->ctrl = new Controls();
        this->ctrl->addItem(new ControlItem(Button::A, "OK"));
        this->ctrl->addItem(new ControlItem(Button::B, "Back"));
        this->addElement(this->ctrl);

        bool hasDate = false;
        int x = 0;
        // Determine which spinners to add
        this->day = nullptr;
        if (d & DTFlag::Day) {
            hasDate = true;
            this->day = new Spinner(x, 0);
            this->day->setWrapAround(true);
            this->day->setDigits(2);
            this->day->setMin(1);
            this->day->setMax(5);
            this->day->setValue(this->refTm.tm_mday);
            this->day->setLabel("Day");
            this->addElement(this->day);
            x += this->day->w();
            x += 40;
        }

        // Add first divide
        this->div1 = nullptr;
        if (this->day != nullptr) {
            this->div1 = new Text(x, 0, "/", SEP_FONT_SIZE);
            this->addElement(this->div1);
        }

        this->month = nullptr;
        if (d & DTFlag::Month) {
            hasDate = true;
            this->month = new Spinner(x, 0);
            this->month->setWrapAround(true);
            this->month->setDigits(2);
            this->month->setMin(1);
            this->month->setMax(12);
            this->month->setValue(this->refTm.tm_mon + 1);
            this->month->setLabel("Month");
            this->addElement(this->month);
            x += this->month->w();
            x += 40;
        }

        // Add first (or second) divide
        this->div2 = nullptr;
        if (this->day != nullptr || this->month != nullptr) {
            this->div2 = new Text(x, 0, "/", SEP_FONT_SIZE);
            this->addElement(this->div2);
        }

        this->year = nullptr;
        if (d & DTFlag::Year) {
            hasDate = true;
            this->year = new Spinner(x, 0, 140);
            this->year->setMin(2000);
            this->year->setMax(2060);
            this->year->setValue(this->refTm.tm_year + 1900);
            this->year->setLabel("Year");
            this->addElement(this->year);
            x += this->year->w();
            x += 40;
        }

        // Increase next x if there is at least one date spinner
        if (hasDate) {
            x += 40;
        }

        this->hour = nullptr;
        if (d & DTFlag::Hour) {
            this->hour = new Spinner(x, 0);
            this->hour->setWrapAround(true);
            this->hour->setDigits(2);
            this->hour->setMin(0);
            this->hour->setMax(23);
            this->hour->setValue(this->refTm.tm_hour);
            this->hour->setLabel("Hour");
            this->addElement(this->hour);
            x += this->hour->w();
            x += 40;
        }

        // Add first colon
        this->col1 = nullptr;
        if (this->hour != nullptr) {
            this->col1 = new Text(x, 0, ":", SEP_FONT_SIZE);
            this->addElement(this->col1);
        }

        this->min = nullptr;
        if (d & DTFlag::Hour) {
            this->min = new Spinner(x, 0);
            this->min->setWrapAround(true);
            this->min->setDigits(2);
            this->min->setMin(0);
            this->min->setMax(59);
            this->min->setValue(this->refTm.tm_min);
            this->min->setLabel("Minute");
            this->addElement(this->min);
            x += this->min->w();
            x += 40;
        }

        // Add first (or second) colon
        this->col2 = nullptr;
        if (this->hour != nullptr || this->min != nullptr) {
            this->col2 = new Text(x, 0, ":", SEP_FONT_SIZE);
            this->addElement(this->col2);
        }

        this->sec = nullptr;
        if (d & DTFlag::Hour) {
            this->sec = new Spinner(x, 0);
            this->sec->setWrapAround(true);
            this->sec->setDigits(2);
            this->sec->setMin(0);
            this->sec->setMax(59);
            this->sec->setValue(this->refTm.tm_sec);
            this->sec->setLabel("Second");
            this->addElement(this->sec);
            x += this->sec->w();
            x += 40;
        }

        // OK Button
        x += 40;
        this->button = new BorderButton(x, 0, 160, 60, 3, "OK", 22, [this](){
            this->setTmValues();
            this->close(true);
        });
        x += this->button->w();
        this->addElement(this->button);

        // Now center everything
        int nx = (this->rect->w() - x)/2;
        int y = this->top->y() + (this->bottom->y() - this->top->y())/2;
        if (this->day != nullptr) {
            this->day->setX(this->day->x() + nx);
            this->day->setY(y - this->day->h()/2);
        }
        if (this->month != nullptr) {
            this->month->setX(this->month->x() + nx);
            this->month->setY(y - this->month->h()/2);
        }
        if (this->year != nullptr) {
            this->year->setX(this->year->x() + nx);
            this->year->setY(y - this->year->h()/2);
        }
        if (this->hour != nullptr) {
            this->hour->setX(this->hour->x() + nx);
            this->hour->setY(y - this->hour->h()/2);
        }
        if (this->min != nullptr) {
            this->min->setX(this->min->x() + nx);
            this->min->setY(y - this->min->h()/2);
        }
        if (this->sec != nullptr) {
            this->sec->setX(this->sec->x() + nx);
            this->sec->setY(y - this->sec->h()/2);
        }
        if (this->div1 != nullptr) {
            this->div1->setX(this->div1->x() - 20 - this->div1->w()/2 + nx);
            this->div1->setY(y - this->div1->h()/2 - 26);
        }
        if (this->div2 != nullptr) {
            this->div2->setX(this->div2->x() - 20 - this->div2->w()/2 + nx);
            this->div2->setY(y - this->div2->h()/2 - 26);
        }
        if (this->col1 != nullptr) {
            this->col1->setX(this->col1->x() - 20 - this->col1->w()/2 + nx);
            this->col1->setY(y - this->col1->h()/2 - 26);
        }
        if (this->col2 != nullptr) {
            this->col2->setX(this->col2->x() - 20 - this->col2->w()/2 + nx);
            this->col2->setY(y - this->col2->h()/2 - 26);
        }
        this->button->setX(this->button->x() + nx);
        this->button->setY(y - this->button->h()/2 - 26);

        // Close without updating
        this->onButtonPress(Button::B, [this](){
            this->close(true);
        });
        // Close and update
        this->onButtonPress(Button::A, [this](){
            this->setTmValues();
            this->close(true);
        });
    }

    void DateTime::setTmValues() {
        if (this->year != nullptr) {
            this->refTm.tm_year = this->year->value() - 1900;
        }
        if (this->month != nullptr) {
            this->refTm.tm_mon = this->month->value() - 1;
        }
        if (this->day != nullptr) {
            this->refTm.tm_mday = this->day->value();
        }
        if (this->hour != nullptr) {
            this->refTm.tm_hour = this->hour->value();
        }
        if (this->min != nullptr) {
            this->refTm.tm_min = this->min->value();
        }
        if (this->sec != nullptr) {
            this->refTm.tm_sec = this->sec->value();
        }
    }

    bool DateTime::handleEvent(InputEvent * e) {
        bool b = Overlay::handleEvent(e);

        // Adjust max days based on current chosen month
        // Done in event call to avoud being done every frame
        if (this->day != nullptr) {
            int d = 31;
            if (this->month != nullptr) {
                if (this->month->value() == 2) {
                    if (this->year->value() % 4 == 0) {
                        d = 29;
                    } else {
                        d = 28;
                    }
                } else if (this->month->value() == 3 || this->month->value() == 5 || this->month->value() == 8 || this->month->value() == 10) {
                        d = 30;
                }
                this->day->setMax(d);
            }
        }

        return b;
    }

    Colour DateTime::getBackgroundColour() {
        return this->rect->getColour();
    }

    void DateTime::setBackgroundColour(Colour c) {
        this->rect->setColour(c);
    }

    Colour DateTime::getInactiveColour() {
        if (this->day != nullptr) {
            return this->day->getArrowColour();
        }
        if (this->month != nullptr) {
            return this->month->getArrowColour();
        }
        if (this->year != nullptr) {
            return this->year->getArrowColour();
        }
        if (this->hour != nullptr) {
            return this->hour->getArrowColour();
        }
        if (this->min != nullptr) {
            return this->min->getArrowColour();
        }
        if (this->sec != nullptr) {
            return this->sec->getArrowColour();
        }

        return Colour{255, 255, 255, 255};
    }

    void DateTime::setInactiveColour(Colour c) {
        if (this->day != nullptr) {
            this->day->setArrowColour(c);
        }
        if (this->month != nullptr) {
            this->month->setArrowColour(c);
        }
        if (this->year != nullptr) {
            this->year->setArrowColour(c);
        }
        if (this->hour != nullptr) {
            this->hour->setArrowColour(c);
        }
        if (this->min != nullptr) {
            this->min->setArrowColour(c);
        }
        if (this->sec != nullptr) {
            this->sec->setArrowColour(c);
        }
    }

    Colour DateTime::getHighlightColour() {
        if (this->day != nullptr) {
            return this->day->getHighlightColour();
        }
        if (this->month != nullptr) {
            return this->month->getHighlightColour();
        }
        if (this->year != nullptr) {
            return this->year->getHighlightColour();
        }
        if (this->hour != nullptr) {
            return this->hour->getHighlightColour();
        }
        if (this->min != nullptr) {
            return this->min->getHighlightColour();
        }
        if (this->sec != nullptr) {
            return this->sec->getHighlightColour();
        }

        return Colour{255, 255, 255, 255};
    }

    void DateTime::setHighlightColour(Colour c) {
        if (this->day != nullptr) {
            this->day->setHighlightColour(c);
        }
        if (this->month != nullptr) {
            this->month->setHighlightColour(c);
        }
        if (this->year != nullptr) {
            this->year->setHighlightColour(c);
        }
        if (this->hour != nullptr) {
            this->hour->setHighlightColour(c);
        }
        if (this->min != nullptr) {
            this->min->setHighlightColour(c);
        }
        if (this->sec != nullptr) {
            this->sec->setHighlightColour(c);
        }
    }

    Colour DateTime::getSeparatorColour() {
        if (this->div1 != nullptr) {
            return this->div1->getColour();
        }
        if (this->div2 != nullptr) {
            return this->div2->getColour();
        }
        if (this->col1 != nullptr) {
            return this->col1->getColour();
        }
        if (this->col2 != nullptr) {
            return this->col2->getColour();
        }
        return Colour{255, 255, 255, 255};
    }

    void DateTime::setSeparatorColour(Colour c) {
        if (this->div1 != nullptr) {
            this->div1->setColour(c);
        }
        if (this->div2 != nullptr) {
            this->div2->setColour(c);
        }
        if (this->col1 != nullptr) {
            this->col1->setColour(c);
        }
        if (this->col2 != nullptr) {
            this->col2->setColour(c);
        }
    }

    Colour DateTime::getTextColour() {
        return this->button->getTextColour();
    }

    void DateTime::setTextColour(Colour c) {
        if (this->day != nullptr) {
            this->day->setTextColour(c);
        }
        if (this->month != nullptr) {
            this->month->setTextColour(c);
        }
        if (this->year != nullptr) {
            this->year->setTextColour(c);
        }
        if (this->hour != nullptr) {
            this->hour->setTextColour(c);
        }
        if (this->min != nullptr) {
            this->min->setTextColour(c);
        }
        if (this->sec != nullptr) {
            this->sec->setTextColour(c);
        }
        this->button->setBorderColour(c);
        this->button->setTextColour(c);
        this->title->setColour(c);
        this->top->setColour(c);
        this->bottom->setColour(c);
        this->ctrl->setColour(c);
    }

    void DateTime::setAllColours(Colour c1, Colour c2, Colour c3, Colour c4, Colour c5) {
        this->setBackgroundColour(c1);
        this->setHighlightColour(c2);
        this->setInactiveColour(c3);
        this->setSeparatorColour(c4);
        this->setTextColour(c5);
    }
};