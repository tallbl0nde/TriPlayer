#include "Application.hpp"
#include "ui/screen/Home.hpp"
#include "ui/screen/Splash.hpp"

namespace Main {
    Application::Application() {
        // Prepare theme
        this->theme_ = new Theme();

        // Open database
        this->database_ = new Database();
        this->database_->migrate();

        // Create sysmodule object (will attempt connection)
        this->sysmodule_ = new Sysmodule();
        // Continue in another thread
        this->sysThread = std::async(std::launch::async, &Sysmodule::process, this->sysmodule_);

        // Create Aether instance
        Aether::ThreadPool::setMaxThreads(8);
        this->display = new Aether::Display();
        this->display->setBackgroundColour(0, 0, 0);
        this->display->setFont("romfs:/Quicksand.ttf");
        this->display->setHighlightColours(Aether::Colour{255, 255, 255, 0}, this->theme_->selected());
        this->display->setHighlightAnimation(Aether::Theme::Dark.highlightFunc);
        this->display->setFadeIn();
        this->display->setShowFPS(true);

        // Setup screens
        this->scSplash = new Screen::Splash(this);
        this->scHome = new Screen::Home(this);
        this->setScreen(ScreenID::Splash);
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

            case Home:
                this->display->setScreen(this->scHome);
                break;
        }
    }

    void Application::pushScreen() {
        this->display->pushScreen();
    }

    void Application::popScreen() {
        this->display->popScreen();
    }

    void Application::lockDatabase() {
        this->database_->close();
        this->sysmodule_->waitRequestDBLock();
        this->database_->openReadWrite();
    }

    void Application::unlockDatabase() {
        this->database_->close();
        this->sysmodule_->sendReleaseDBLock();
        this->database_->openReadOnly();
    }

    Database * Application::database() {
        return this->database_;
    }

    Sysmodule * Application::sysmodule() {
        return this->sysmodule_;
    }

    Theme * Application::theme() {
        return this->theme_;
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
        delete this->scHome;
        delete this->scSplash;

        // Cleanup Aether after screens are deleted
        delete this->display;

        // Disconnect from sysmodule
        this->sysmodule_->exit();
        this->sysThread.get();
        delete this->sysmodule_;

        // Close/save database
        delete this->database_;
    }
};