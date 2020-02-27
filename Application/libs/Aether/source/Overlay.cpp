#include "Overlay.hpp"

// Background colour
#define BG_COLOUR Colour{0, 0, 0, 150}

namespace Aether {
    Overlay::Overlay() : Screen() {
        this->close_ = false;
    }

    void Overlay::close(bool b) {
        this->close_ = b;
    }

    bool Overlay::shouldClose() {
        return this->close_;
    }

    void Overlay::render() {
        // Draw background
        SDLHelper::drawFilledRect(BG_COLOUR, this->x(), this->y(), this->w(), this->h());

        // Draw elements
        Screen::render();
    }
};