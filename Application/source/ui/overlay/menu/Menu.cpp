#include "ui/overlay/menu/Menu.hpp"

// Width of background
#define WIDTH 380

namespace CustomOvl::Menu {
    Menu::Menu(Type t) : Aether::Overlay() {
        this->type = t;

        // Close if B pressed
        this->onButtonPress(Aether::Button::B, [this]() {
            this->close();
        });

        // Add a second transparent layer cause we like it dark
        Aether::Rectangle * r = new Aether::Rectangle(this->x(), this->y(), this->w(), this->h());
        r->setColour(Aether::Colour{0, 0, 0, 120});
        this->addElement(r);

        // Background (will be resized vertically by derived classes)
        this->bg = new Aether::Rectangle(0, 0, WIDTH, 1, 25);
        this->addElement(this->bg);

        // Create line texture
        this->line = SDLHelper::renderFilledRect(this->bg->w() - 40, 1);
        this->lineColour = Aether::Colour{255, 255, 255, 255};
    }

    void Menu::setBackgroundColour(Aether::Colour c) {
        this->bg->setColour(c);
    }

    void Menu::setLineColour(Aether::Colour c) {
        this->lineColour = c;
    }

    bool Menu::handleEvent(Aether::InputEvent * e) {
        if (Overlay::handleEvent(e)) {
            return true;
        }

        if (e->type() == Aether::EventType::TouchReleased) {
            if (e->touchX() < this->bg->x() || e->touchX() > this->bg->x() + this->bg->w() || e->touchY() < this->bg->y() || e->touchY() > this->bg->y() + this->bg->h()) {
                this->close();
                return true;
            }
        }

        return false;
    }

    Menu::~Menu() {
        SDLHelper::destroyTexture(this->line);
    }
};