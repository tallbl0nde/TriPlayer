#include "BaseProgress.hpp"

namespace Aether {
    BaseProgress::BaseProgress(int x, int y, int w, int h) : Element(x, y, w, h) {
        this->value_ = 0.0;
    }

    float BaseProgress::value() {
        return this->value_;
    }

    void BaseProgress::setValue(float v) {
        if (v < 0) {
            v = 0;
        } else if (v > 100) {
            v = 100;
        }
        this->value_ = v;
    }
}