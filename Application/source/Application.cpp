#include "Application.hpp"

namespace Main {
    Application::Application() {
        // Create Aether instance
        this->display = new Aether::Display();
        this->display->setBackgroundColour(20, 20, 35);
        this->display->setFont("romfs:/Quicksand.ttf");
        this->display->setHighlightColours(Aether::Colour{255, 255, 255, 0}, Aether::Theme::Dark.selected);
        this->display->setHighlightAnimation(Aether::Theme::Dark.highlightFunc);
        this->display->setShowFPS(true);

        // Setup screens
        this->scSplash = new Screen::Splash(this);
        this->scMain = new Screen::MainScreen(this);

        this->setScreen(ScreenID::Main);
    }

    void Application::setHoldDelay(int i) {
        this->display->setHoldDelay(i);
    }

    void Application::addOverlay(Aether::Overlay * o) {
        this->display->addOverlay(o);
    }

    void Application::setScreen(ScreenID s) {
        switch (s) {
            case Splash:
                this->display->setScreen(this->scSplash);
                break;

            case Main:
                this->display->setScreen(this->scMain);
                break;
        }
    }

    void Application::pushScreen() {
        this->display->pushScreen();
    }

    void Application::popScreen() {
        this->display->popScreen();
    }


    void Application::run() {
        // Do main loop
        while (this->display->loop()) {

        }
    }

    void Application::exit() {
        this->display->exit();
    }

    Application::~Application() {
        // Delete screens
        delete this->scMain;
        delete this->scSplash;

        // Cleanup Aether
        delete this->display;
    }
};