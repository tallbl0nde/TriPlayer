#include "InputEvent.hpp"
#include "utils/Utils.hpp"

namespace Aether {
    InputEvent::InputEvent(SDL_Event e) {
        // Turn all directional events into dpad
        Button tmp = Utils::SDLtoButton(e.jbutton.button);
        if (e.type == SDL_JOYBUTTONDOWN || e.type == SDL_JOYBUTTONUP) {
            if (tmp == Button::LSTICK_LEFT || tmp == Button::RSTICK_LEFT) {
                tmp = Button::DPAD_LEFT;
            } else if (tmp == Button::LSTICK_RIGHT || tmp == Button::RSTICK_RIGHT) {
                tmp = Button::DPAD_RIGHT;
            } else if (tmp == Button::LSTICK_UP || tmp == Button::RSTICK_UP) {
                tmp = Button::DPAD_UP;
            } else if (tmp == Button::LSTICK_DOWN || tmp == Button::RSTICK_DOWN) {
                tmp = Button::DPAD_DOWN;
            }
        }

        switch (e.type) {
            // Button events
            case SDL_JOYBUTTONDOWN:
                this->type_ = ButtonPressed;
                this->button_ = tmp;
                this->id_ = e.jbutton.which;
                this->touchX_ = 0;
                this->touchY_ = 0;
                this->touchDX_ = 0;
                this->touchDY_ = 0;
                break;
            case SDL_JOYBUTTONUP:
                this->type_ = ButtonReleased;
                this->button_ = tmp;
                this->id_ = e.jbutton.which;
                this->touchX_ = 0;
                this->touchY_ = 0;
                this->touchDX_ = 0;
                this->touchDY_ = 0;
                break;

            // Touch events
            case SDL_FINGERDOWN:
                this->type_ = TouchPressed;
                this->button_ = Button::NO_BUTTON;
                this->id_ = -1;
                this->touchX_ = e.tfinger.x * 1280;
                this->touchY_ = e.tfinger.y * 720;
                this->touchDX_ = 0;
                this->touchDY_ = 0;
                break;
            case SDL_FINGERMOTION:
                this->type_ = TouchMoved;
                this->button_ = Button::NO_BUTTON;
                this->id_ = -1;
                this->touchX_ = e.tfinger.x * 1280;
                this->touchY_ = e.tfinger.y * 720;
                this->touchDX_ = e.tfinger.dx * 1280;
                this->touchDY_ = e.tfinger.dy * 720;
                break;
            case SDL_FINGERUP:
                this->type_ = TouchReleased;
                this->button_ = Button::NO_BUTTON;
                this->id_ = -1;
                this->touchX_ = e.tfinger.x * 1280;
                this->touchY_ = e.tfinger.y * 720;
                this->touchDX_ = e.tfinger.dx * 1280;
                this->touchDY_ = e.tfinger.dy * 720;
                break;
        }
    }

    EventType InputEvent::type() {
        return this->type_;
    }

    Button InputEvent::button() {
        return this->button_;
    }

    int InputEvent::id() {
        return this->id_;
    }

    int InputEvent::touchX() {
        return this->touchX_;
    }

    int InputEvent::touchY() {
        return this->touchY_;
    }

    int InputEvent::touchDX() {
        return this->touchDX_;
    }

    int InputEvent::touchDY() {
        return this->touchDY_;
    }

    InputEvent::~InputEvent() {

    }
};