#include "Application.hpp"
#include "ui/screen/Screen.hpp"

namespace Screen {
    Screen::Screen(Main::Application * a) : Aether::Screen() {
        this->app = a;
        this->isLoaded = false;

        // Assign L and R to skip based on config
        this->onButtonPress(Aether::Button::L, [this]() {
            if (this->app->config()->skipWithLR()) {
                this->app->sysmodule()->sendPrevious();
            }
        });
        this->onButtonPress(Aether::Button::R, [this]() {
            if (this->app->config()->skipWithLR()) {
                this->app->sysmodule()->sendNext();
            }
        });
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