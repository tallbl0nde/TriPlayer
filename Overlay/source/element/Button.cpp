#include "element/Button.hpp"

namespace Element {
    Button::Button(u16 x, u16 y, u16 w, u16 h) : tsl::elm::Element() {
        this->setBoundaries(x, y, w, h);
    }

    void Button::draw(tsl::gfx::Renderer * renderer) {
        renderer->drawRect(ELEMENT_BOUNDS(this), 0xffff);
    }

    void Button::layout(u16 parentX, u16 parentY, u16 parentW, u16 parentH) {
        // Do nothing yet?
    }
};