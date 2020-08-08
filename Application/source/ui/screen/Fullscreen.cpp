#include "Application.hpp"
#include "ui/screen/Fullscreen.hpp"
#include "utils/Splash.hpp"
#include "utils/Utils.hpp"

// Diameter of buttons
#define BTN_SIZE 50
// Max width of song text
#define MAX_TEXT_WIDTH 600

namespace Screen {
    Fullscreen::Fullscreen(Main::Application * a) : Screen() {
        this->app = a;

        // Close this screen when B is pressed
        this->onButtonPress(Aether::Button::B, [this]() {
            this->app->popScreen();
        });
    }

    void Fullscreen::setColours() {
        this->title->setColour(this->primary);
        this->artist->setColour(this->secondary);
        this->previous->setColour(this->primary);
        this->play->setColour(this->primary);
        this->pause->setColour(this->primary);
        this->next->setColour(this->primary);
        this->position->setColour(this->primary);
        this->duration->setColour(this->primary);
        this->seekBar->setBarBackgroundColour(this->tertiary);
        this->seekBar->setBarForegroundColour(this->secondary);
        this->seekBar->setKnobColour(this->primary);
    }

    void Fullscreen::updateImage(const std::string & path) {
        // Remove old image first
        this->removeElement(this->albumArt);

        // Now create a surface and get colours
        SDL_Surface * image = SDLHelper::renderImageS(path);
        Utils::Splash::Palette palette = Utils::Splash::getPaletteForSurface(image);
        if (!palette.invalid) {
            // Set matching colours if valid
            if (palette.bgLight) {
                this->primary = palette.background;
                this->secondary = Utils::Splash::changeLightness(this->primary, -20);
                this->tertiary = Utils::Splash::changeLightness(this->secondary, -20);
            } else {
                this->primary = palette.primary;
                this->secondary = palette.secondary;
                this->tertiary = Utils::Splash::changeLightness(this->secondary, -10);
                this->tertiary.a = 200;
            }
            palette.background.a = (palette.bgLight ? 150 : 255);
            this->gradient->setColour(palette.background);

        } else {
            // Otherwise set defaults
            this->gradient->setColour(Aether::Colour{150, 255, 255, 255});
            this->primary = this->app->theme()->FG();
            this->secondary = this->app->theme()->muted();
            this->tertiary = this->app->theme()->muted2();
        }

        // Add image and set colours
        this->albumArt = new CustomElm::Image(460, 65, image);
        this->albumArt->setWH(360, 360);
        this->addElement(this->albumArt);
        this->setColours();
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
                this->repeat->setColour(this->tertiary);
                this->repeatOneC->setHidden(true);
                if (this->controls->focussed() == this->repeatOneContainer) {
                    this->controls->setFocussed(this->repeatContainer);
                }
                break;

            case RepeatMode::One:
                this->repeatOneC->setHidden(false);
                this->repeatOne->setColour(this->secondary);
                this->repeatC->setHidden(true);
                if (this->controls->focussed() == this->repeatContainer) {
                    this->controls->setFocussed(this->repeatOneContainer);
                }
                break;

            case RepeatMode::All:
                this->repeatC->setHidden(false);
                this->repeat->setColour(this->secondary);
                this->repeatOneC->setHidden(true);
                if (this->controls->focussed() == this->repeatOneContainer) {
                    this->controls->setFocussed(this->repeatContainer);
                }
                break;
        }

        // Ensure shuffle has correct colour
        if (this->app->sysmodule()->shuffleMode() == ShuffleMode::On) {
            this->shuffle->setColour(this->secondary);
        } else {
            this->shuffle->setColour(this->tertiary);
        }

        // Update song metadata
        SongID id = this->app->sysmodule()->currentSong();
        if (id != this->playingID) {
            this->playingID = id;
            Metadata::Song m = this->app->database()->getSongMetadataForID(id);
            if (m.ID != -1) {
                this->title->setString(m.title);
                if (this->title->texW() > MAX_TEXT_WIDTH) {
                    this->title->setW(MAX_TEXT_WIDTH);
                }
                this->title->setX(640 - this->title->w()/2);
                this->artist->setString(m.artist);
                if (this->artist->texW() > MAX_TEXT_WIDTH) {
                    this->artist->setW(MAX_TEXT_WIDTH);
                }
                this->artist->setX(640 - this->artist->w()/2);
                this->duration->setString(Utils::secondsToHMS(m.duration));
                this->durationVal = m.duration;
            }

            // Change album cover
            Metadata::Album md = this->app->database()->getAlbumMetadataForID(this->app->database()->getAlbumIDForSong(m.ID));
            this->updateImage(md.imagePath.empty() ? "romfs:/misc/noalbum.png" : md.imagePath);
        }

        // Update the seekbar
        if (!this->seekBar->selected()) {
            this->seekBar->setValue(this->app->sysmodule()->position());
        }
        this->position->setString(Utils::secondsToHMS(this->durationVal * (this->seekBar->value() / 100.0)));
        this->position->setX(465 - this->position->w());

        // Now update elements
        Screen::update(dt);
    }

    void Fullscreen::onLoad() {
        // Add background images
        this->bg = new Aether::Image(0, 0, "romfs:/bg/main.png");
        this->addElement(this->bg);
        this->gradient = new Aether::Image(0, 0, "romfs:/bg/gradient.png");
        this->addElement(this->gradient);

        // === METADATA ===
        this->title = new Aether::Text(0, 450, "", 36);
        this->title->setScroll(true);
        this->title->setScrollSpeed(35);
        this->title->setScrollWaitTime(1200);
        this->addElement(this->title);
        this->artist = new Aether::Text(0, this->title->y() + 50, "", 24);
        this->addElement(this->artist);

        // === CONTROLS ===
        this->controls = new Aether::Container(300, 590-BTN_SIZE/2, 680, BTN_SIZE);
        // Shuffle
        this->shuffle = new Aether::Image(0, 0, "romfs:/icons/shuffle.png");
        this->shuffleC = new CustomElm::RoundButton(480 - BTN_SIZE/2, 590 - BTN_SIZE/2, BTN_SIZE);
        this->shuffleC->setImage(this->shuffle);
        this->shuffleC->setCallback([this]() {
            this->app->sysmodule()->sendSetShuffle((this->app->sysmodule()->shuffleMode() == ShuffleMode::Off ? ShuffleMode::On : ShuffleMode::Off));
        });
        this->controls->addElement(this->shuffleC);

        // Previous
        this->previous = new Aether::Image(0, 0, "romfs:/icons/previous.png");
        this->previousC = new CustomElm::RoundButton(560 - BTN_SIZE/2, 590 - BTN_SIZE/2, BTN_SIZE);
        this->previousC->setImage(this->previous);
        this->previousC->setCallback([this]() {
            this->app->sysmodule()->sendPrevious();
        });
        this->controls->addElement(this->previousC);

        // Play/pause
        this->play = new Aether::Image(0, 0, "romfs:/icons/playsmall.png");
        this->playC = new CustomElm::RoundButton(640 - BTN_SIZE/2, 590 - BTN_SIZE/2, BTN_SIZE);
        this->playC->setHidden(true);
        this->playC->setImage(this->play);
        this->playC->setCallback([this]() {
            this->app->sysmodule()->sendResume();
        });
        this->controls->addElement(this->playC);
        this->pause = new Aether::Image(0, 0, "romfs:/icons/pausesmall.png");
        this->pauseC = new CustomElm::RoundButton(640 - BTN_SIZE/2, 590 - BTN_SIZE/2, BTN_SIZE);
        this->pauseC->setImage(this->pause);
        this->pauseC->setCallback([this]() {
            this->app->sysmodule()->sendPause();
        });
        this->controls->addElement(this->pauseC);
        this->controls->setFocussed(this->pauseC);

        // Next
        this->next = new Aether::Image(0, 0, "romfs:/icons/next.png");
        this->nextC = new CustomElm::RoundButton(720 - BTN_SIZE/2, 590 - BTN_SIZE/2, BTN_SIZE);
        this->nextC->setImage(this->next);
        this->nextC->setCallback([this]() {
            this->app->sysmodule()->sendNext();
        });
        this->controls->addElement(this->nextC);

        // Repeat
        this->repeatContainer = new Aether::Container(770, 610, 100, 60);
        this->repeat = new Aether::Image(0, 0, "romfs:/icons/repeat.png");
        this->repeatC = new CustomElm::RoundButton(800 - BTN_SIZE/2, 590 - BTN_SIZE/2, BTN_SIZE);
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
        this->repeatOneContainer = new Aether::Container(770, 600, 100, 80);
        this->repeatOne = new Aether::Image(0, 0, "romfs:/icons/repeatone.png");
        this->repeatOneC = new CustomElm::RoundButton(800 - BTN_SIZE/2, 590 - BTN_SIZE/2, BTN_SIZE);
        this->repeatOneC->setCallback([this]() {
            this->app->sysmodule()->sendSetRepeat(RepeatMode::All);
        });
        this->repeatOneC->setImage(this->repeatOne);
        this->repeatOneContainer->addElement(this->repeatOneC);
        this->controls->addElement(this->repeatOneContainer);

        this->addElement(this->controls);

        // === SEEKBAR ===
        this->position = new Aether::Text(0, 0, "0:00", 18);
        this->position->setY(658 - this->position->h()/2);
        this->addElement(this->position);
        this->seekBar = new CustomElm::Slider(490, 649, 300, 20, 8);
        this->seekBar->setNudge(1);
        this->seekBar->setCallback([this]() {
            this->app->sysmodule()->sendSetPosition(this->seekBar->value());
        });
        this->addElement(this->seekBar);
        this->duration = new Aether::Text(815, 0, "0:00", 18);
        this->duration->setY(658 - this->duration->h()/2);
        this->addElement(this->duration);

        this->albumArt = nullptr;
        this->buttonMs = 0;
        this->playingID = -1;
    }

    void Fullscreen::onUnload() {
        // Remove added elements
        this->removeElement(this->position);
        this->removeElement(this->seekBar);
        this->removeElement(this->duration);
        this->removeElement(this->controls);
        this->removeElement(this->artist);
        this->removeElement(this->title);
        this->removeElement(this->albumArt);
        this->removeElement(this->bg);
        this->removeElement(this->gradient);
    }
};