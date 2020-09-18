#include "Paths.hpp"
#include "ui/element/Player.hpp"
#include "utils/Utils.hpp"

// Clip if artist name is longer than this
#define ARTIST_MAX_WIDTH 200
// Scroll if song name is longer than this
#define TITLE_MAX_WIDTH 280

// Diameter of play/pause button
#define BTN_PLAYPAUSE_SIZE 60
// Diameter of 'main' buttons
#define BTN_MAIN_SIZE 50

namespace CustomElm {
    Player::Player() : Aether::Container(0, 590, 1280, 130) {
        this->accent = Aether::Colour{255, 255, 255, 255};
        this->muted = Aether::Colour{255, 255, 255, 255};
        this->playerBg = new Aether::Rectangle(this->x(), this->y(), this->w(), this->h());
        this->addElement(this->playerBg);

        // Album/song playing
        this->albumCover = new Aether::Image(10, 600, Path::App::DefaultArtFile);
        this->albumCover->setWH(110, 110);
        this->addElement(this->albumCover);
        this->trackName = new Aether::Text(140, 625, "", 24);
        this->trackName->setScroll(true);
        this->trackName->setScrollSpeed(35);
        this->trackName->setScrollWaitTime(1200);
        this->addElement(this->trackName);
        this->trackArtist = new Aether::Text(140, 660, "", 18);
        this->addElement(this->trackArtist);
        this->trackArtistDots = new Aether::Text(0, 0, "...", 18);
        this->trackArtistDots->setHidden(true);
        this->addElement(this->trackArtistDots);

        // Controls
        this->shuffle = new Aether::Image(0, 0, "romfs:/icons/shuffle.png");
        this->shuffleC = new CustomElm::RoundButton(490 - BTN_MAIN_SIZE/2, 640 - BTN_MAIN_SIZE/2, BTN_MAIN_SIZE);
        this->shuffleC->setImage(this->shuffle);
        this->addElement(this->shuffleC);
        this->previous = new Aether::Image(0, 0, "romfs:/icons/previous.png");
        this->previousC = new CustomElm::RoundButton(560 - BTN_MAIN_SIZE/2, 640 - BTN_MAIN_SIZE/2, BTN_MAIN_SIZE);
        this->previousC->setImage(this->previous);
        this->addElement(this->previousC);
        this->play = new Aether::Image(0, 0, "romfs:/icons/play.png");
        this->playC = new CustomElm::RoundButton(640 - BTN_PLAYPAUSE_SIZE/2, 640 - BTN_PLAYPAUSE_SIZE/2, BTN_PLAYPAUSE_SIZE);
        this->playC->setHidden(true);
        this->playC->setImage(this->play);
        this->addElement(this->playC);
        this->pause = new Aether::Image(0, 0, "romfs:/icons/pause.png");
        this->pauseC = new CustomElm::RoundButton(640 - BTN_PLAYPAUSE_SIZE/2, 640 - BTN_PLAYPAUSE_SIZE/2, BTN_PLAYPAUSE_SIZE);
        this->pauseC->setImage(this->pause);
        this->addElement(this->pauseC);
        this->setFocussed(this->pauseC);
        this->next = new Aether::Image(0, 0, "romfs:/icons/next.png");
        this->nextC = new CustomElm::RoundButton(720 - BTN_MAIN_SIZE/2, 640 - BTN_MAIN_SIZE/2, BTN_MAIN_SIZE);
        this->nextC->setImage(this->next);
        this->addElement(this->nextC);
        this->repeatContainer = new Aether::Container(760, 600, 100, 80);
        this->repeat = new Aether::Image(0, 0, "romfs:/icons/repeat.png");
        this->repeatC = new CustomElm::RoundButton(790 - BTN_MAIN_SIZE/2, 640 - BTN_MAIN_SIZE/2, BTN_MAIN_SIZE);
        this->repeatC->setCallback([this]() {
            Aether::Colour c1 = this->repeat->getColour();
            Aether::Colour c2 = this->accent;
            if (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a) {
                this->repeatFunc(RepeatMode::Off);
            } else {
                this->repeatFunc(RepeatMode::One);
            }
        });
        this->repeatC->setImage(this->repeat);
        this->repeatContainer->addElement(this->repeatC);
        this->addElement(this->repeatContainer);
        this->repeatOneContainer = new Aether::Container(760, 600, 100, 80);
        this->repeatOne = new Aether::Image(0, 0, "romfs:/icons/repeatone.png");
        this->repeatOneC = new CustomElm::RoundButton(790 - BTN_MAIN_SIZE/2, 640 - BTN_MAIN_SIZE/2, BTN_MAIN_SIZE);
        this->repeatOneC->setCallback([this]() {
            this->repeatFunc(RepeatMode::All);
        });
        this->repeatOneC->setImage(this->repeatOne);
        this->repeatOneContainer->addElement(this->repeatOneC);
        this->addElement(this->repeatOneContainer);

        // Seeking
        this->position = new Aether::Text(0, 0, "0:00", 18);
        this->position->setX(420 - this->position->w());
        this->position->setY(693 - this->position->h()/2);
        this->addElement(this->position);
        this->seekBar = new CustomElm::Slider(440, 684, 400, 20, 8);
        this->seekBar->setNudge(1);
        this->addElement(this->seekBar);
        this->duration = new Aether::Text(860, 0, "0:00", 18);
        this->duration->setY(693 - this->duration->h()/2);
        this->addElement(this->duration);

        // Volume + full screen
        this->volumeIcon = new Aether::Image(0, 0, "romfs:/icons/volume.png");
        this->volumeIconC = new CustomElm::RoundButton(985 - BTN_MAIN_SIZE/2, 653 - BTN_MAIN_SIZE/2, BTN_MAIN_SIZE);
        this->volumeIconC->setImage(this->volumeIcon);
        this->addElement(this->volumeIconC);
        this->volumeIconMuted = new Aether::Image(0, 0, "romfs:/icons/volumemuted.png");
        this->volumeIconMutedC = new CustomElm::RoundButton(985 - BTN_MAIN_SIZE/2, 653 - BTN_MAIN_SIZE/2, BTN_MAIN_SIZE);
        this->volumeIconMutedC->setImage(this->volumeIconMuted);
        this->addElement(this->volumeIconMutedC);
        this->volume = new CustomElm::Slider(1025, 644, 130, 20, 8);
        this->volume->setNudge(5);
        this->addElement(this->volume);
        this->fullscreen = new Aether::Image(1195, 638, "romfs:/icons/fullscreen.png");
        this->fullscreenC = new CustomElm::RoundButton(1212 - BTN_MAIN_SIZE/2, 655 - BTN_MAIN_SIZE/2, BTN_MAIN_SIZE);
        this->fullscreenC->addElement(this->fullscreen);
        this->addElement(this->fullscreenC);
    }

    void Player::setAlbumCover(Aether::Image * i) {
        this->removeElement(this->albumCover);
        this->albumCover = i;
        if (i != nullptr) {
            this->albumCover->setXY(10, 600);
            this->albumCover->setWH(110, 110);
            this->addElement(this->albumCover);
        }
    }

    void Player::setTrackName(std::string str) {
        // Scroll song name if too long
        this->trackName->setString(str);
        if (this->trackName->w() > TITLE_MAX_WIDTH) {
            this->trackName->setW(TITLE_MAX_WIDTH);
        }
    }

    void Player::setTrackArtist(std::string str) {
        // Artist is just cut off (for now?)
        this->trackArtist->setString(str);
        this->trackArtistDots->setHidden(true);
        if (this->trackArtist->w() > ARTIST_MAX_WIDTH) {
            this->trackArtist->setW(ARTIST_MAX_WIDTH);
            this->trackArtistDots->setXY(this->trackArtist->x() + ARTIST_MAX_WIDTH, this->trackArtist->y());
            this->trackArtistDots->setHidden(false);
        }
    }

    void Player::setShuffle(bool shuffled) {
        this->shuffle->setColour((shuffled ? this->accent : this->muted));
    }

    void Player::setPlaying(bool playing) {
        // Hide/unhide elements
        if (playing) {
            this->pauseC->setHidden(false);
            this->playC->setHidden(true);
            if (this->focussed() == this->playC) {
                this->setFocussed(this->pauseC);
            }

        } else {
            this->pauseC->setHidden(true);
            this->playC->setHidden(false);
            if (this->focussed() == this->pauseC) {
                this->setFocussed(this->playC);
            }
        }
    }

    void Player::setRepeat(RepeatMode r) {
        switch (r) {
            case RepeatMode::Off:
                this->repeatC->setHidden(false);
                this->repeat->setColour(this->muted);
                this->repeatOneC->setHidden(true);
                if (this->focussed() == this->repeatOneContainer) {
                    this->setFocussed(this->repeatContainer);
                }
                break;

            case RepeatMode::One:
                this->repeatOneC->setHidden(false);
                this->repeatOne->setColour(this->accent);
                this->repeatC->setHidden(true);
                if (this->focussed() == this->repeatContainer) {
                    this->setFocussed(this->repeatOneContainer);
                }
                break;

            case RepeatMode::All:
                this->repeatC->setHidden(false);
                this->repeat->setColour(this->accent);
                this->repeatOneC->setHidden(true);
                if (this->focussed() == this->repeatOneContainer) {
                    this->setFocussed(this->repeatContainer);
                }
                break;
        }
    }

    void Player::setPosition(double pos) {
        if (!this->seekBar->selected()) {
            this->setPosition_(pos);
        }
    }

    void Player::setPosition_(double pos) {
        this->position->setString(Utils::secondsToHMS(this->durationVal * (pos / 100.0)));
        this->position->setX(420 - this->position->w());
        this->seekBar->setValue(pos);
    }

    void Player::setDuration(unsigned int d) {
        this->durationVal = d;
        this->duration->setString(Utils::secondsToHMS(d));
    }

    void Player::setVolume(double vol) {
        if (!this->volume->selected()) {
            this->volume->setValue(vol);
        }

        // Show appropriate volume icon
        if (vol == 0.0) {
            this->volumeIconC->setHidden(true);
            this->volumeIconMutedC->setHidden(false);
            if (this->focussed() == this->volumeIconC) {
                this->setFocussed(this->volumeIconMutedC);
            }
        } else {
            this->volumeIconC->setHidden(false);
            this->volumeIconMutedC->setHidden(true);
            if (this->focussed() == this->volumeIconMutedC) {
                this->setFocussed(this->volumeIconC);
            }
        }
    }

    void Player::setAccentColour(Aether::Colour c) {
        this->accent = c;
        this->seekBar->setBarForegroundColour(c);
        this->volume->setBarForegroundColour(c);
    }

    void Player::setBackgroundColour(Aether::Colour c) {
        this->playerBg->setColour(c);
        this->shuffleC->setBackgroundColour(c);
        this->previousC->setBackgroundColour(c);
        this->playC->setBackgroundColour(c);
        this->pauseC->setBackgroundColour(c);
        this->nextC->setBackgroundColour(c);
        this->repeatC->setBackgroundColour(c);
        this->repeatOneC->setBackgroundColour(c);
        this->volumeIconC->setBackgroundColour(c);
        this->volumeIconMutedC->setBackgroundColour(c);
        this->fullscreenC->setBackgroundColour(c);
    }

    void Player::setForegroundColour(Aether::Colour c) {
        this->duration->setColour(c);
        this->next->setColour(c);
        this->position->setColour(c);
        this->previous->setColour(c);
        this->play->setColour(c);
        this->pause->setColour(c);
        this->seekBar->setKnobColour(c);
        this->trackName->setColour(c);
        this->volumeIcon->setColour(c);
        this->volumeIconMuted->setColour(c);
        this->volume->setKnobColour(c);
    }

    void Player::setMutedColour(Aether::Colour c) {
        this->muted = c;
        this->fullscreen->setColour(c);
        this->shuffle->setColour(c);
        this->repeat->setColour(c);
        this->repeatOne->setColour(c);
        this->trackArtist->setColour(c);
        this->trackArtistDots->setColour(c);
    }

    void Player::setMuted2Colour(Aether::Colour c) {
        this->seekBar->setBarBackgroundColour(c);
        this->volume->setBarBackgroundColour(c);
    }

    void Player::setShuffleCallback(std::function<void()> f) {
        this->shuffleC->setCallback(f);
    }

    void Player::setPreviousCallback(std::function<void()> f) {
        this->previousC->setCallback(f);
    }

    void Player::setPauseCallback(std::function<void()> f) {
        this->pauseC->setCallback(f);
    }

    void Player::setPlayCallback(std::function<void()> f) {
        this->playC->setCallback(f);
    }

    void Player::setNextCallback(std::function<void()> f) {
        this->nextC->setCallback(f);
    }

    void Player::setRepeatCallback(std::function<void(RepeatMode)> f) {
        this->repeatFunc = f;
    }

    void Player::setSeekCallback(std::function<void(float)> f) {
        this->seekBar->setCallback([this, f]() {
            f(this->seekBar->value());
        });
    }

    void Player::setVolumeIconCallback(std::function<void(bool)> f) {
        this->volumeIconC->setCallback([this, f]() {
            f(false);
        });
        this->volumeIconMutedC->setCallback([this, f]() {
            f(true);
        });
    }

    void Player::setVolumeCallback(std::function<void(float)> f) {
        this->volumeFunc = f;
        this->volume->setCallback([this]() {
            this->volumeFunc(this->volume->value());
        });
    }

    void Player::setFullscreenCallback(std::function<void()> f) {
        this->fullscreenC->setCallback(f);
    }

    void Player::update(uint32_t dt) {
        // Update seek bar position
        this->setPosition_(this->seekBar->value());

        // Update volume instantly
        if (this->volume->selected()) {
            this->volumeFunc(this->volume->value());
        }

        Container::update(dt);
    }
};