#include "Screen.hpp"

// Size of highlight border
#define HIGHLIGHT_SIZE 5

namespace Aether {
    Screen::Screen() : Container(0, 0, 1280, 720) {
        // Init all callbacks to nullptr to indicate none set
        for (int i = Button::A; i < Button::SR_RIGHT; i++) {
            this->pressFuncs[static_cast<Button>(i)] = nullptr;
            this->releaseFuncs[static_cast<Button>(i)] = nullptr;
        }
    }

    void Screen::onLoad() {

    }

    void Screen::onUnload() {

    }

    void Screen::onButtonPress(Button k, std::function<void()> f) {
        this->pressFuncs[k] = f;
    }

    void Screen::onButtonRelease(Button k, std::function<void()> f) {
        this->releaseFuncs[k] = f;
    }

    bool Screen::handleEvent(InputEvent * e) {
        // Check for callback and execute if there is one
        if (e->type() == EventType::ButtonPressed) {
            if (this->pressFuncs[e->button()] != nullptr) {
                this->pressFuncs[e->button()]();
                return true;
            }
        } else if (e->type() == EventType::ButtonReleased) {
            if (this->releaseFuncs[e->button()] != nullptr) {
                this->releaseFuncs[e->button()]();
                return true;
            }
        }

        // If no callback continue down the chain
        return Container::handleEvent(e);
    }
};