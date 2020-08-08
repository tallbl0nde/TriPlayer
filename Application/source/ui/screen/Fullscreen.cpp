#include "Application.hpp"
#include "ui/screen/Fullscreen.hpp"
#include "utils/Splash.hpp"
#include "utils/Utils.hpp"

// Diameter of buttons
#define BTN_SIZE 50
// Max width of song text
#define MAX_TEXT_WIDTH 600

// Time to fade between songs
#define ANIM_TIME 1500
// Time to fade in in milliseconds
#define FADE_IN_TIME 300
// Time to fade out in milliseconds
#define FADE_OUT_TIME 600
// Number of milliseconds to show highlight for
#define HI_TIMEOUT 7000

namespace Screen {
    Fullscreen::Fullscreen(Main::Application * a) : Screen() {
        this->app = a;

        // Close this screen when B is pressed
        this->onButtonPress(Aether::Button::B, [this]() {
            this->app->popScreen();
        });
    }

    Aether::Colour Fullscreen::highlightColour(uint32_t t) {
        // Get the original highlight colour
        Aether::Colour colour = Aether::Theme::Dark.highlightFunc(t);

        // Adjust the alpha based on time since button press
        if (this->buttonMs < 0) {
            // Fade in
            double per = (this->buttonMs/(double)-FADE_IN_TIME);
            colour.a = 255 - 255*per;

        } else if (this->buttonMs > HI_TIMEOUT) {
            // Fade out
            int val = (this->buttonMs-HI_TIMEOUT > FADE_OUT_TIME ? FADE_OUT_TIME : this->buttonMs-HI_TIMEOUT);
            double per = (val/(double)FADE_OUT_TIME);
            colour.a = 255 - 255*per;
        }

        return colour;
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
        // Move old image into vector
        if (this->albumArt != nullptr) {
            this->oldAlbumArt.push_back(this->albumArt);
        }
        this->oldBackground = this->currentBackground;
        this->interpolatePos = 0.0d;

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
            this->targetBackground = palette.background;

        } else {
            // Otherwise set defaults
            this->primary = this->app->theme()->FG();
            this->secondary = this->app->theme()->muted();
            this->tertiary = this->app->theme()->muted2();
            this->targetBackground = Aether::Colour{150, 255, 255, 255};
        }

        // Add image and set colours
        this->albumArt = new CustomElm::Image(460, 65, image);
        this->albumArt->setWH(360, 360);
        this->albumArt->setColour(Aether::Colour{255, 255, 255, 0});
        this->addElement(this->albumArt);
        this->setColours();
    }

    bool Fullscreen::handleEvent(Aether::InputEvent * e) {
        // Reset timer when button pressed and don't actually do anything else
        if (e->type() == Aether::EventType::ButtonPressed) {
            // If we were hidden make sure to unhide
            if (this->buttonMs > HI_TIMEOUT) {
                if (this->buttonMs > HI_TIMEOUT + FADE_OUT_TIME) {
                    this->buttonMs = -FADE_IN_TIME;
                } else {
                    int val = (this->buttonMs > HI_TIMEOUT+FADE_OUT_TIME ? FADE_OUT_TIME : this->buttonMs - HI_TIMEOUT);
                    this->buttonMs = -FADE_IN_TIME*(val/(double)FADE_OUT_TIME);
                }
                return true;
            }

            // Otherwise just reset timer
            this->buttonMs = 0;

        // Also reset the timer on any touch events
        } else if (e->type() == Aether::EventType::TouchPressed || e->type() == Aether::EventType::TouchReleased) {
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

        // Fade out old album images
        if (!this->oldAlbumArt.empty()) {
            size_t i = 0;
            while (i < this->oldAlbumArt.size()) {
                // Check if we need to delete the image as it is transparent
                Aether::Colour colour = this->oldAlbumArt[i]->getColour();
                int alpha = colour.a;
                alpha -= (255 * (dt/(double)ANIM_TIME));
                if (alpha <= 0) {
                    this->removeElement(this->oldAlbumArt[i]);
                    this->oldAlbumArt.erase(this->oldAlbumArt.begin() + i);
                    continue;
                }

                // Otherwise the image can still be seen and should continue fading
                colour.a = static_cast<uint8_t>(alpha);
                this->oldAlbumArt[i]->setColour(colour);
                i++;
            }
        }

        // Fade in new image
        if (this->albumArt != nullptr) {
            Aether::Colour colour = this->albumArt->getColour();
            if (colour.a < 255) {
                int alpha = colour.a;
                alpha += (255 * (dt/(double)ANIM_TIME));
                if (alpha > 255) {
                    alpha = 255;
                }
                colour.a = static_cast<uint8_t>(alpha);
                this->albumArt->setColour(colour);
            }
        }

        // Interpolate the colour of the background gradient
        if (this->interpolatePos < 1.0d) {
            this->interpolatePos += (dt/(double)ANIM_TIME);
            this->currentBackground = Utils::Splash::interpolateColours(this->oldBackground, this->targetBackground, this->interpolatePos);
            this->gradient->setColour(this->currentBackground);
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
        if (!this->isTouch) {
            this->buttonMs += dt;
        }
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

        // This screen determines the highlight animation colour
        this->app->setHighlightAnimation([this](uint32_t t) {
            return this->highlightColour(t);
        });

        // Initialize variables
        this->albumArt = nullptr;
        this->interpolatePos = 0.0d;
        this->currentBackground = Aether::Colour{0, 0, 0, 255};
        this->targetBackground = this->currentBackground;
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
        for (size_t i = 0; i < this->oldAlbumArt.size(); i++) {
            this->removeElement(this->oldAlbumArt[i]);
        }
        this->oldAlbumArt.clear();

        // Reset highlight animation
        this->app->setHighlightAnimation(nullptr);
    }
};