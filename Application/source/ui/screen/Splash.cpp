#include "Splash.hpp"

namespace Screen {
    Splash::Splash(Main::Application * a) : Screen() {
        this->app = a;

        // Aloow to exit for now
        this->onButtonPress(Aether::Button::B, [this](){
            this->app->exit();
        });
    }

    void Splash::onLoad() {
        // Set background
        this->bg = new Aether::Image(0, 0, "romfs:/bg/splash.png");
        this->addElement(this->bg);

        // Add all other elements
        Aether::Text * t = new Aether::Text(560, 315, "Version " + std::string(VER_STRING), 26);
        t->setColour(Aether::Colour{255, 255, 255, 255});
        this->addElement(t);

        this->status = new Aether::Text(640, 520, "Scanning your library...", 26);
        this->status->setX(640 - this->status->w()/2);
        this->status->setColour(Aether::Colour{255, 255, 255, 255});
        this->addElement(this->status);

        this->statusNum = new Aether::Text(640, 560, "File 0 of 0", 20);
        this->statusNum->setX(640 - this->statusNum->w()/2);
        this->statusNum->setColour(Aether::Colour{255, 255, 255, 255});
        this->addElement(this->statusNum);

        this->pbar = new Aether::RoundProgressBar(400, 605, 480, 10);
        this->pbar->setValue(0.0);
        this->pbar->setBackgroundColour(Aether::Colour{55, 55, 55, 255});
        this->pbar->setForegroundColour(Aether::Colour{0, 255, 255, 255});
        this->addElement(this->pbar);

        this->percent = new Aether::Text(this->pbar->x() + this->pbar->w() + 20, 610, "0%", 20);
        this->percent->setY(610 - this->percent->h()/2);
        this->addElement(this->percent);

        this->anim = new Aether::Animation(this->pbar->x() - 60, 600, 40, 20);
        for (size_t i = 1; i <= 50; i++) {
            Aether::Image * im = new Aether::Image(this->anim->x(), this->anim->y(), "romfs:/anim/infload/" + std::to_string(i) + ".png");
            im->setWH(40, 20);
            im->setColour(Aether::Colour{0, 255, 255, 255});
            this->anim->addElement(im);
        }
        anim->setAnimateSpeed(50);
        this->addElement(this->anim);

        this->hint = new Aether::Text(640, 685, "Please don't quit until the scan is complete!", 18);
        this->hint->setX(640 - this->hint->w()/2);
        this->hint->setColour(Aether::Colour{70, 70, 70, 255});
        this->addElement(this->hint);

        // Most stuff is hidden until a list of files is found

    }

    void Splash::onUnload() {
        this->removeElement(this->bg);
        this->removeElement(this->status);
        this->removeElement(this->statusNum);
        this->removeElement(this->pbar);
        this->removeElement(this->percent);
        this->removeElement(this->anim);
    }
};