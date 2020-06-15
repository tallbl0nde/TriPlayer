#include "Player.hpp"
#include "Utils.hpp"

// Clip if artist name is longer than this
#define ARTIST_MAX_WIDTH 200
// Scroll if song name is longer than this
#define TITLE_MAX_WIDTH 280

namespace CustomElm {
    Player::Player() : Aether::Container(0, 590, 1280, 130) {
        this->accent = Aether::Colour{255, 255, 255, 255};
        this->muted = Aether::Colour{255, 255, 255, 255};
        this->playerBg = new Aether::Rectangle(this->x(), this->y(), this->w(), this->h());
        this->addElement(this->playerBg);

        // Album/song playing
        this->albumCover = nullptr;
        this->albumCoverDefault = new Aether::Image(10, 600, "romfs:/misc/noalbum.png");
        this->albumCoverDefault->setWH(110, 110);
        this->addElement(this->albumCoverDefault);
        this->trackName = new Aether::Text(140, 625, "Nothing playing!", 24);
        this->trackName->setScroll(true);
        this->trackName->setScrollSpeed(35);
        this->trackName->setScrollWaitTime(1200);
        this->addElement(this->trackName);
        this->trackArtist = new Aether::Text(140, 660, "Play a song", 18);
        this->addElement(this->trackArtist);
        this->trackArtistDots = new Aether::Text(0, 0, "...", 18);
        this->addElement(this->trackArtistDots);

        // Controls
        this->shuffle = new Aether::Image(480, 630, "romfs:/icons/shuffle.png");
        this->addElement(this->shuffle);
        this->previous = new Aether::Image(550, 628, "romfs:/icons/previous.png");
        this->addElement(this->previous);
        this->play = new Aether::Image(610, 610, "romfs:/icons/play.png");
        this->play->setHidden(true);
        this->addElement(this->play);
        this->pause = new Aether::Image(610, 610, "romfs:/icons/pause.png");
        this->addElement(this->pause);
        this->next = new Aether::Image(705, 628, "romfs:/icons/next.png");
        this->addElement(this->next);
        this->repeat = new Aether::Image(770, 630, "romfs:/icons/repeat.png");
        this->repeat->setCallback([this]() {
            Aether::Colour c1 = this->repeat->getColour();
            Aether::Colour c2 = this->accent;
            if (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a) {
                this->repeatFunc(RepeatMode::Off);
            } else {
                this->repeatFunc(RepeatMode::One);
            }
        });
        this->addElement(this->repeat);
        this->repeatOne = new Aether::Image(770, 630, "romfs:/icons/repeatone.png");
        this->repeatOne->setCallback([this]() {
            this->repeatFunc(RepeatMode::All);
        });
        this->addElement(this->repeatOne);

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
        this->volumeIcon = new Aether::Image(970, 638, "romfs:/icons/volume.png");
        this->addElement(this->volumeIcon);
        this->volume = new CustomElm::Slider(1025, 644, 130, 20, 8);
        this->volume->setNudge(5);
        this->addElement(this->volume);
        this->fullscreen = new Aether::Image(1195, 638, "romfs:/icons/fullscreen.png");
        this->addElement(this->fullscreen);
    }

    void Player::setAlbumCover(unsigned char * data, size_t size) {
        this->removeElement(this->albumCover);
        this->albumCover = nullptr;
        if (data != nullptr) {
            this->albumCover = new Aether::Image(this->albumCoverDefault->x(), this->albumCoverDefault->y(), data, size);
            this->albumCover->setWH(this->albumCoverDefault->w(), this->albumCoverDefault->h());
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
            this->pause->setHidden(false);
            this->play->setHidden(true);
            if (this->play->focussed()) {
                this->setFocussed(this->pause);
            }

        } else {
            this->pause->setHidden(true);
            this->play->setHidden(false);
            if (this->pause->focussed()) {
                this->setFocussed(this->play);
            }
        }
    }

    void Player::setRepeat(RepeatMode r) {
        switch (r) {
            case RepeatMode::Off:
                this->repeat->setHidden(false);
                this->repeat->setColour(this->muted);
                this->repeatOne->setHidden(true);
                if (this->repeatOne->focussed()) {
                    this->setFocussed(this->repeat);
                }
                break;

            case RepeatMode::One:
                this->repeatOne->setHidden(false);
                this->repeatOne->setColour(this->accent);
                this->repeat->setHidden(true);
                if (this->repeat->focussed()) {
                    this->setFocussed(this->repeatOne);
                }
                break;

            case RepeatMode::All:
                this->repeat->setHidden(false);
                this->repeat->setColour(this->accent);
                this->repeatOne->setHidden(true);
                if (this->repeatOne->focussed()) {
                    this->setFocussed(this->repeat);
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
    }

    void Player::setAccentColour(Aether::Colour c) {
        this->accent = c;
        this->seekBar->setBarForegroundColour(c);
        this->volume->setBarForegroundColour(c);
    }

    void Player::setBackgroundColour(Aether::Colour c) {
        this->playerBg->setColour(c);
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
        this->shuffle->setCallback(f);
    }

    void Player::setPreviousCallback(std::function<void()> f) {
        this->previous->setCallback(f);
    }

    void Player::setPauseCallback(std::function<void()> f) {
        this->pause->setCallback(f);
    }

    void Player::setPlayCallback(std::function<void()> f) {
        this->play->setCallback(f);
    }

    void Player::setNextCallback(std::function<void()> f) {
        this->next->setCallback(f);
    }

    void Player::setRepeatCallback(std::function<void(RepeatMode)> f) {
        this->repeatFunc = f;
    }

    void Player::setSeekCallback(std::function<void(float)> f) {
        this->seekBar->setCallback([this, f]() {
            f(this->seekBar->value());
        });
    }

    void Player::setVolumeIconCallback(std::function<void()> f) {
        this->volumeIcon->setCallback(f);
    }

    void Player::setVolumeCallback(std::function<void(float)> f) {
        this->volume->setCallback([this, f]() {
            f(this->volume->value());
        });
    }

    void Player::setFullscreenCallback(std::function<void()> f) {
        this->fullscreen->setCallback(f);
    }

    void Player::update(uint32_t dt) {
        this->setPosition_(this->seekBar->value());
    }
};