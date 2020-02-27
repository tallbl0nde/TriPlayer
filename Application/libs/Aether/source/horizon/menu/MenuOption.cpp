#include "MenuOption.hpp"

// Default dimensions
#define DEFAULT_WIDTH 300
#define DEFAULT_HEIGHT 70
#define DEFAULT_FONT_SIZE 22

namespace Aether {
    MenuOption::MenuOption(std::string s, Colour a, Colour ia, std::function<void()> f) : Element() {
        // Create and add child elements
        this->rect = new Rectangle(8, 9, 4, 52);
        this->text = new Text(24, 0, s, DEFAULT_FONT_SIZE);
        this->text->setScroll(true);
        this->addElement(this->rect);
        this->addElement(this->text);

        // Set size (width is handled when added to menu)
        this->setH(DEFAULT_HEIGHT);

        // Position text
        this->text->setY(this->y() + this->h()/2 - this->text->texH()/2);

        this->setActive(false);
        this->setCallback(f);

        this->setActiveColour(a);
        this->setInactiveColour(ia);
    }

    void MenuOption::setW(int w) {
        Element::setW(w);
        if (this->text->w() > (this->x() + this->w()) - this->text->x()) {
            this->text->setW((this->x() + this->w()) - this->text->x());
        }
    }

    void MenuOption::setActive(bool b) {
        if (b) {
            this->rect->setHidden(false);
            this->text->setColour(this->activeColour);
        } else {
            this->rect->setHidden(true);
            this->text->setColour(this->inactiveColour);
        }

        this->active = b;
    }

    void MenuOption::setActiveColour(Colour c) {
        this->activeColour = c;
        this->rect->setColour(c);
        this->setActive(this->active);
    }

    void MenuOption::setInactiveColour(Colour c) {
        this->inactiveColour = c;
        this->setActive(this->active);
    }
};