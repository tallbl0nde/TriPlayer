#include "MP3.hpp"
#include "Splash.hpp"
#include "Utils.hpp"

namespace Screen {
    Splash::Splash(Main::Application * a) : Screen() {
        this->app = a;

        // Allow to exit for now
        this->onButtonPress(Aether::Button::B, [this](){
            this->app->exit();
        });
    }

    void Splash::processFiles() {
        // Search for files in folder
        this->currentStage = Search;
        this->currentFile = 0;
        std::vector<std::string> files = Utils::getFilesWithExt("/music", ".mp3");
        this->totalFiles = files.size();

        // Find songs that need to be removed
        std::vector<SongID> remove;
        for (size_t i = 0; i < files.size(); i++) {
            // Check if modified since added to database
            unsigned int dataMTime = this->app->database()->getModifiedTimeForPath(files[i]);
            unsigned int diskMTime = Utils::getModifiedTimestamp(files[i]);

            // If so remove previous entry and then add later
            if (diskMTime > dataMTime) {
                remove.push_back(this->app->database()->getSongIDForPath(files[i]));
            }
        }

        std::vector<std::string> paths = this->app->database()->getAllSongPaths();
        for (size_t i = 0; i < paths.size(); i++) {
            // If file is no longer present remove it
            if (std::find(files.begin(), files.end(), paths[i]) == files.end()) {
                remove.push_back(this->app->database()->getSongIDForPath(paths[i]));
            }
        }

        // Find songs that need to be added
        std::vector< std::pair<std::string, unsigned int> > add;
        for (size_t i = 0; i < files.size(); i++) {
            // Check if modified since added to database
            unsigned int dataMTime = this->app->database()->getModifiedTimeForPath(files[i]);
            unsigned int diskMTime = Utils::getModifiedTimestamp(files[i]);

            // Add part of 'update entry'
            if (dataMTime == 0 || diskMTime > dataMTime) {
                add.push_back(std::pair<std::string, unsigned int>(files[i], diskMTime));
            }
        }

        // Don't do anything if no changes
        if (remove.size() == 0 && add.size() == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(600));
            this->currentStage = Done;
            return;
        }

        // Proceed to parsing new songs
        this->currentStage = Parse;
        std::vector<SongInfo> infos;
        for (size_t i = 0; i < add.size(); i++) {
            infos.push_back(Utils::MP3::getInfoFromID3(add[i].first));
            this->currentFile = i+1;
        }
        this->currentFile++;

        // Now commit all changes to DB!
        this->currentStage = Update;
        // Reset the sysmodule to stop playback, clear queue and ensure DB won't be accessed
        this->app->sysmodule()->sendReset();
        // Lock the database for writing
        this->app->database()->lock();

        // First remove entries
        for (size_t i = 0; i < remove.size(); i++) {
            this->app->database()->removeSong(remove[i]);
        }
        // Next add entries (infos and add have same size)
        for (size_t i = 0; i < add.size(); i++) {
            this->app->database()->addSong(infos[i], add[i].first, add[i].second);
        }

        // Clean 'empty' artists/albums from DB
        this->app->database()->cleanup();

        // Unlock database now that everything is done
        this->app->database()->unlock();
        this->currentStage = Done;
    }

    void Splash::update(uint32_t dt) {
        Screen::update(dt);

        // Read into copy in case value changes
        LoadingStage curr = this->currentStage;
        if (curr != this->lastStage) {
            switch (curr) {
                case Search:
                    // Never called
                    break;

                case Parse:
                    this->status->setString("Scanning new songs...");
                    this->status->setX(640 - this->status->w()/2);
                    this->statusNum->setHidden(false);
                    this->anim->setX(this->pbar->x() - 60);
                    this->pbar->setHidden(false);
                    this->percent->setHidden(false);
                    this->anim->setHidden(false);
                    this->hint->setHidden(false);
                    break;

                case Update:
                    this->status->setString("Updating database...");
                    this->statusNum->setHidden(true);
                    this->anim->setX(620);
                    this->pbar->setHidden(true);
                    this->percent->setHidden(true);
                    break;

                case Done:
                    this->anim->setHidden(true);
                    this->hint->setHidden(true);
                    this->status->setHidden(true);
                    this->app->setScreen(Main::ScreenID::Main);
                    break;
            }
            this->lastStage = curr;
        }

        if (curr == Parse) {
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
        this->currentStage = Search;
        this->lastStage = Search;

        // Check if connected to sysmodule
        if (!this->app->sysmodule()->error()) {
            // Start searching for files
            this->future = std::async(std::launch::async, &Splash::processFiles, this);
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