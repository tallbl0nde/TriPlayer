#include "ui/screen/Screen.hpp"

namespace Screen {
    Screen::Screen(Main::Application * a) : Aether::Screen() {
        this->app = a;
        this->isLoaded = false;
    }

    void Screen::updateColours() {

    }

    void Screen::onLoad() {
        this->isLoaded = true;
    }

    void Screen::onUnload() {
        this->isLoaded = false;
    }
};