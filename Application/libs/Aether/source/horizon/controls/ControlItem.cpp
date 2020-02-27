#include <algorithm>
#include "ControlItem.hpp"
#include "utils/Utils.hpp"

// Font sizes
#define HINT_FONT_SIZE 22
#define ICON_FONT_SIZE 25

// "Padding" around actual textures
#define PADDING 20
// Gap between icon + hint
#define TEXT_GAP 13

// "Map" of buttons to unicode characters
const std::string buttonChar[] = {
    "\uE0E0", // A
    "\uE0E1", // B
    "\uE0E2", // X
    "\uE0E3", // Y
    "\uE0F7", // LSTICK
    "\uE0F8", // RSTICK
    "\uE0E4", // L
    "\uE0E5", // R
    "\uE0E6", // ZL
    "\uE0E7", // ZR
    "\uE0EF", // PLUS
    "\uE0F0", // MINUS
    "\uE0ED", // DPAD_LEFT
    "\uE0EB", // DPAD_UP
    "\uE0EE", // DPAD_RIGHT
    "\uE0EC", // DPAD_DOWN
    "", // LSTICK_LEFT
    "", // LSTICK_UP
    "", // LSTICK_RIGHT
    "", // LSTICK_DOWN
    "", // RSTICK_LEFT
    "", // RSTICK_UP
    "", // RSTICK_RIGHT
    "", // RSTICK_DOWN
    "\uE0E8", // SL_LEFT
    "\uE0E9", // SR_LEFT
    "\uE0E8", // SL_RIGHT
    "\uE0E9"  // SR_RIGHT
};

namespace Aether {
    ControlItem::ControlItem(Button k, std::string s) : Element() {
        // Create and add elements
        this->icon = new Text(PADDING, 0, buttonChar[k], ICON_FONT_SIZE, FontType::Extended);
        this->hint = new Text(0, 0, s, HINT_FONT_SIZE);
        this->addElement(this->icon);
        this->addElement(this->hint);

        // Set size of this element
        this->setW(PADDING*2 + this->icon->w() + TEXT_GAP + this->hint->w());
        this->setH(std::max(this->icon->h(), this->hint->h()) + PADDING*2);

        // Situate children
        this->hint->setX(this->icon->x() + this->icon->w() + TEXT_GAP);
        this->icon->setY(this->y() + (this->h() - this->icon->h())/2);
        this->hint->setY(this->y() + (this->h() - this->hint->h())/2);

        // Adding a callback automatically makes this item selectable and touchable
        this->setCallback([k](){
            // Send pushed event
            SDL_Event event;
            event.type = SDL_JOYBUTTONDOWN;
            event.jbutton.which = FAKE_ID;
            event.jbutton.button = (uint8_t)k;
            event.jbutton.state = SDL_PRESSED;
            SDL_PushEvent(&event);
            // Send released event (so basically a verrry fast button press)
            SDL_Event event2;
            event2.type = SDL_JOYBUTTONUP;
            event2.jbutton.which = FAKE_ID;
            event2.jbutton.button = (uint8_t)k;
            event2.jbutton.state = SDL_RELEASED;
            SDL_PushEvent(&event2);
        });
        this->setSelectable(false);
    }

    Colour ControlItem::getColour() {
        return this->colour;
    }

    void ControlItem::setColour(Colour c) {
        this->colour = c;
        this->icon->setColour(c);
        this->hint->setColour(c);
    }

    void ControlItem::setColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        this->setColour(Colour{r, g, b, a});
    }
};