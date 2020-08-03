#include "Application.hpp"
#include "ui/screen/Fullscreen.hpp"

// Diameter of play/pause button
#define BTN_PLAYPAUSE_SIZE 60
// Diameter of 'main' buttons
#define BTN_MAIN_SIZE 50

namespace Screen {
    Fullscreen::Fullscreen(Main::Application * a) : Screen() {
        this->app = a;

        // Close this screen when B is pressed
        this->onButtonPress(Aether::Button::B, [this]() {
            this->app->popScreen();
        });
    }

    bool Fullscreen::handleEvent(Aether::InputEvent * e) {
        // Reset timer when button pressed
        if (e->type() == Aether::EventType::ButtonPressed) {
            this->buttonMs = 0;
        }

        return Screen::handleEvent(e);
    }

    void Fullscreen::update(uint32_t dt) {
        // Update the playing status
        PlaybackStatus ps = this->app->sysmodule()->status();
        if (ps == PlaybackStatus::Error) {
            // Handle error

        } else if (ps == PlaybackStatus::Playing) {
            this->pauseC->setHidden(false);
            this->playC->setHidden(true);
            if (this->controls->focussed() == this->playC) {
                this->controls->setFocussed(this->pauseC);
            }

        } else {
            this->pauseC->setHidden(true);
            this->playC->setHidden(false);
            if (this->controls->focussed() == this->pauseC) {
                this->controls->setFocussed(this->playC);
            }
        }

        // Ensure repeat icon matches and has correct colour
        RepeatMode rm = this->app->sysmodule()->repeatMode();
        switch (rm) {
            case RepeatMode::Off:
                this->repeatC->setHidden(false);
                this->repeat->setColour(this->app->theme()->muted());
                this->repeatOneC->setHidden(true);
                if (this->controls->focussed() == this->repeatOneContainer) {
                    this->controls->setFocussed(this->repeatContainer);
                }
                break;

            case RepeatMode::One:
                this->repeatOneC->setHidden(false);
                this->repeatOne->setColour(this->app->theme()->accent());
                this->repeatC->setHidden(true);
                if (this->controls->focussed() == this->repeatContainer) {
                    this->controls->setFocussed(this->repeatOneContainer);
                }
                break;

            case RepeatMode::All:
                this->repeatC->setHidden(false);
                this->repeat->setColour(this->app->theme()->accent());
                this->repeatOneC->setHidden(true);
                if (this->controls->focussed() == this->repeatOneContainer) {
                    this->controls->setFocussed(this->repeatContainer);
                }
                break;
        }

        // Ensure shuffle has correct colour
        if (this->app->sysmodule()->shuffleMode() == ShuffleMode::On) {
            this->shuffle->setColour(this->app->theme()->accent());

        } else {
            this->shuffle->setColour(this->app->theme()->muted());
        }

        // this->player->setPosition(this->app->sysmodule()->position());

        // Update song metadata
        SongID id = this->app->sysmodule()->currentSong();
        if (id != this->playingID) {
            this->playingID = id;
            Metadata::Song m = this->app->database()->getSongMetadataForID(id);
            if (m.ID != -1) {
                this->title->setString(m.title);
                this->title->setX(640 - this->title->w()/2);
                this->artist->setString(m.artist);
                this->artist->setX(640 - this->artist->w()/2);
                // this->player->setDuration(m.duration);
            }

            // Change album cover
            Metadata::Album md = this->app->database()->getAlbumMetadataForID(this->app->database()->getAlbumIDForSong(m.ID));
            this->removeElement(this->albumArt);
            this->albumArt = new Aether::Image(460, 65, md.imagePath.empty() ? "romfs:/misc/noalbum.png" : md.imagePath);
            this->albumArt->setWH(360, 360);
            this->addElement(this->albumArt);
        }

        // Now update elements
        Screen::update(dt);
    }

    void Fullscreen::onLoad() {
        // Add background images
        this->bg = new Aether::Image(0, 0, "romfs:/bg/main.png");
        this->addElement(this->bg);
        this->gradient = new Aether::Image(0, 0, "romfs:/bg/gradient.png");
        this->gradient->setColour(Aether::Colour{255, 255, 255, 255});
        this->addElement(this->gradient);

        // === METADATA ===
        this->title = new Aether::Text(0, 460, "", 36);
        this->title->setColour(this->app->theme()->FG());
        this->addElement(this->title);
        this->artist = new Aether::Text(0, this->title->y() + 50, "", 24);
        this->artist->setColour(this->app->theme()->muted());
        this->addElement(this->artist);

        // === CONTROLS ===
        this->controls = new Aether::Container(300, 610, 680, 60);
        // Shuffle
        this->shuffle = new Aether::Image(0, 0, "romfs:/icons/shuffle.png");
        this->shuffleC = new CustomElm::RoundButton(490 - BTN_MAIN_SIZE/2, 640 - BTN_MAIN_SIZE/2, BTN_MAIN_SIZE);
        this->shuffleC->setImage(this->shuffle);
        this->shuffleC->setCallback([this]() {
            this->app->sysmodule()->sendSetShuffle((this->app->sysmodule()->shuffleMode() == ShuffleMode::Off ? ShuffleMode::On : ShuffleMode::Off));
        });
        this->controls->addElement(this->shuffleC);

        // Previous
        this->previous = new Aether::Image(0, 0, "romfs:/icons/previous.png");
        this->previous->setColour(this->app->theme()->FG());
        this->previousC = new CustomElm::RoundButton(560 - BTN_MAIN_SIZE/2, 640 - BTN_MAIN_SIZE/2, BTN_MAIN_SIZE);
        this->previousC->setImage(this->previous);
        this->previousC->setCallback([this]() {
            this->app->sysmodule()->sendPrevious();
        });
        this->controls->addElement(this->previousC);

        // Play/pause
        this->play = new Aether::Image(0, 0, "romfs:/icons/play.png");
        this->play->setColour(this->app->theme()->FG());
        this->playC = new CustomElm::RoundButton(640 - BTN_PLAYPAUSE_SIZE/2, 640 - BTN_PLAYPAUSE_SIZE/2, BTN_PLAYPAUSE_SIZE);
        this->playC->setHidden(true);
        this->playC->setImage(this->play);
        this->playC->setCallback([this]() {
            this->app->sysmodule()->sendResume();
        });
        this->controls->addElement(this->playC);
        this->pause = new Aether::Image(0, 0, "romfs:/icons/pause.png");
        this->pause->setColour(this->app->theme()->FG());
        this->pauseC = new CustomElm::RoundButton(640 - BTN_PLAYPAUSE_SIZE/2, 640 - BTN_PLAYPAUSE_SIZE/2, BTN_PLAYPAUSE_SIZE);
        this->pauseC->setImage(this->pause);
        this->pauseC->setCallback([this]() {
            this->app->sysmodule()->sendPause();
        });
        this->controls->addElement(this->pauseC);
        this->controls->setFocussed(this->pauseC);

        // Next
        this->next = new Aether::Image(0, 0, "romfs:/icons/next.png");
        this->next->setColour(this->app->theme()->FG());
        this->nextC = new CustomElm::RoundButton(720 - BTN_MAIN_SIZE/2, 640 - BTN_MAIN_SIZE/2, BTN_MAIN_SIZE);
        this->nextC->setImage(this->next);
        this->nextC->setCallback([this]() {
            this->app->sysmodule()->sendNext();
        });
        this->controls->addElement(this->nextC);

        // Repeat
        this->repeatContainer = new Aether::Container(760, 610, 100, 60);
        this->repeat = new Aether::Image(0, 0, "romfs:/icons/repeat.png");
        this->repeatC = new CustomElm::RoundButton(790 - BTN_MAIN_SIZE/2, 640 - BTN_MAIN_SIZE/2, BTN_MAIN_SIZE);
        this->repeatC->setImage(this->repeat);
        this->repeatC->setCallback([this]() {
            if (this->app->sysmodule()->repeatMode() == RepeatMode::All) {
                this->app->sysmodule()->sendSetRepeat(RepeatMode::Off);
            } else {
                this->app->sysmodule()->sendSetRepeat(RepeatMode::One);
            }
        });
        this->repeatContainer->addElement(this->repeatC);
        this->controls->addElement(this->repeatContainer);
        this->repeatOneContainer = new Aether::Container(760, 600, 100, 80);
        this->repeatOne = new Aether::Image(0, 0, "romfs:/icons/repeatone.png");
        this->repeatOneC = new CustomElm::RoundButton(790 - BTN_MAIN_SIZE/2, 640 - BTN_MAIN_SIZE/2, BTN_MAIN_SIZE);
        this->repeatOneC->setCallback([this]() {
            this->app->sysmodule()->sendSetRepeat(RepeatMode::All);
        });
        this->repeatOneC->setImage(this->repeatOne);
        this->repeatOneContainer->addElement(this->repeatOneC);
        this->controls->addElement(this->repeatOneContainer);

        this->addElement(this->controls);
        this->albumArt = nullptr;
        this->buttonMs = 0;
        this->playingID = -1;
    }

    void Fullscreen::onUnload() {
        // Remove added elements
        this->removeElement(this->controls);
        this->removeElement(this->artist);
        this->removeElement(this->title);
        this->removeElement(this->albumArt);
    }
};