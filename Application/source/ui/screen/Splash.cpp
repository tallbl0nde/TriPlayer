#include "Splash.hpp"

namespace Screen {
    Splash::Splash(Main::Application * a) : Screen() {
        this->app = a;
    }

    void Splash::onLoad() {
        this->name = new Aether::Text(50, 50, "TriPlayer", 40);
        this->addElement(this->name);
        this->status = new Aether::Text(50, 100, "Loading...", 20);
        this->addElement(this->status);
        this->pbar = new Aether::RoundProgressBar(50, 300, 300);
        this->pbar->setValue(0.0);
        this->pbar->setBackgroundColour(Aether::Theme::Dark.mutedLine);
        this->pbar->setForegroundColour(Aether::Theme::Dark.accent);
        this->addElement(this->pbar);
        this->pbartext = new Aether::Text(370, 300, "0%", 24);
        this->addElement(this->pbartext);
    }

    void Splash::onUnload() {
        this->removeElement(this->name);
        this->removeElement(this->status);
        this->removeElement(this->pbar);
        this->removeElement(this->pbartext);
    }
};