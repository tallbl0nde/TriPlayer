#include "Application.hpp"
#include "Splash.hpp"

namespace Screen {
    Splash::Splash(Main::Application * a) : Screen() {
        this->app = a;

        // Can only exit on an error
        this->onButtonPress(Aether::Button::B, [this](){
            if (this->app->sysmodule()->error()) {
                this->app->exit();
            }
        });
    }

    void Splash::update(uint32_t dt) {
        Screen::update(dt);

        // Read into copy in case value changes
        ProcessStage curr = this->currentStage;
        if (curr != this->lastStage) {
            switch (curr) {
                case ProcessStage::Search:
                    // Never called
                    break;

                case ProcessStage::Parse:
                    this->status->setString("Scanning new songs...");
                    this->status->setX(640 - this->status->w()/2);
                    this->statusNum->setHidden(false);
                    this->anim->setX(this->pbar->x() - 60);
                    this->pbar->setHidden(false);
                    this->percent->setHidden(false);
                    this->anim->setHidden(false);
                    this->hint->setHidden(false);
                    break;

                case ProcessStage::Update:
                    this->status->setString("Updating database...");
                    this->statusNum->setHidden(true);
                    this->anim->setX(620);
                    this->pbar->setHidden(true);
                    this->percent->setHidden(true);
                    break;

                case ProcessStage::Done:
                    this->anim->setHidden(true);
                    this->hint->setHidden(true);
                    this->status->setHidden(true);
                    this->app->setScreen(Main::ScreenID::Main);
                    break;
            }
            this->lastStage = curr;
        }

        if (curr == ProcessStage::Parse) {
            // Read into copy in case value changes
            int currF = this->currentFile;
            if (currF != this->lastFile) {
                // Update progress bar + text
                this->statusNum->setString("File " + std::to_string(currF) + " of " + std::to_string(this->totalFiles));
                this->statusNum->setX(640 - this->statusNum->w()/2);
                this->pbar->setValue(100 * (float)this->currentFile/(this->totalFiles + 1));
                this->percent->setString(Utils::truncateToDecimalPlace(std::to_string(this->pbar->value()), 1) + "%");
                this->lastFile = currF;
            }
        }
    }

    void Splash::onLoad() {
        // Set background
        this->bg = new Aether::Image(0, 0, "romfs:/bg/splash.png");
        this->addElement(this->bg);

        // Add all other elements
        Aether::Text * t = new Aether::Text(560, 315, "Version " + std::string(VER_STRING), 26);
        t->setColour(Aether::Colour{255, 255, 255, 255});
        this->addElement(t);

        this->status = new Aether::Text(640, 520, "Searching for new files...", 26);
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

        this->anim = new Aether::Animation(620, 600, 40, 20);
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
        this->statusNum->setHidden(true);
        this->pbar->setHidden(true);
        this->percent->setHidden(true);
        this->hint->setHidden(true);

        this->currentFile = 0;
        this->lastFile = 0;
        this->currentStage = ProcessStage::Search;
        this->lastStage = ProcessStage::Search;

        // Check if connected to sysmodule
        if (!this->app->sysmodule()->error()) {
            // Start searching for files
            this->future = std::async(std::launch::async, [this](){
                Utils::processFileChanges(this->app->database(), this->app->sysmodule(), this->currentFile, this->currentStage, this->totalFiles);
            });
        } else {
            this->status->setString("Unable to connect to Sysmodule!");
            this->status->setX(640 - this->status->w()/2);
            this->statusNum->setString("Check that it is enabled and up to date");
            this->statusNum->setX(640 - this->statusNum->w()/2);
            this->statusNum->setHidden(false);
            this->anim->setHidden(true);
        }
    }

    void Splash::onUnload() {
        this->removeElement(this->bg);
        this->removeElement(this->status);
        this->removeElement(this->statusNum);
        this->removeElement(this->pbar);
        this->removeElement(this->percent);
        this->removeElement(this->anim);
        this->removeElement(this->hint);
    }
};