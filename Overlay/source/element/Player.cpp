#include <cstring>
#include "element/Button.hpp"
#include "element/Player.hpp"
#include "ipc/TriPlayer.hpp"

// Image files
#include "next_png.h"
#include "no_album_png.h"
#include "pause_png.h"
#include "play_png.h"
#include "previous_png.h"
#include "repeat_one_png.h"
#include "repeat_png.h"
#include "shuffle_png.h"

// Button padding
#define BUTTON_PADDING_X 8
#define BUTTON_PADDING_Y 8

// Font sizes for text
#define ARTIST_FONT_SIZE 20
#define STOP_FONT_SIZE 18
#define TIME_FONT_SIZE 20
#define TITLE_FONT_SIZE 24

namespace Element {
    Player::Player() : tsl::elm::Element() {
        // Read images into memory
        std::vector<uint8_t> buffer;

        // Default album art
        this->defaultArt = false;

        // Shuffle icon
        buffer.resize(shuffle_png_size);
        std::memcpy(&buffer[0], shuffle_png, shuffle_png_size);
        this->shuffle = new Button(BUTTON_PADDING_X, BUTTON_PADDING_Y, buffer);
        this->shuffle->setParent(this);
        this->shuffle->setCallback([this]() {
            TriPlayer::setShuffleMode(this->shuffled ? TriPlayer::Shuffle::Off : TriPlayer::Shuffle::On);
        });

        // Previous icon
        buffer.resize(previous_png_size);
        std::memcpy(&buffer[0], previous_png, previous_png_size);
        this->previous = new Button(BUTTON_PADDING_X, BUTTON_PADDING_Y, buffer);
        this->previous->setParent(this);
        this->previous->setCallback([]() {
            TriPlayer::previous();
        });

        // Play/pause icon
        buffer.resize(play_png_size);
        std::memcpy(&buffer[0], play_png, play_png_size);
        this->play = new Button(BUTTON_PADDING_X, BUTTON_PADDING_Y, buffer);
        this->play->setParent(this);
        buffer.resize(pause_png_size);
        std::memcpy(&buffer[0], pause_png, pause_png_size);
        this->play->addAltImage(buffer);
        this->play->setCallback([this]() {
            if (this->playing) {
                TriPlayer::pause();
            } else {
                TriPlayer::resume();
            }
        });

        // Next icon
        buffer.resize(next_png_size);
        std::memcpy(&buffer[0], next_png, next_png_size);
        this->next = new Button(BUTTON_PADDING_X, BUTTON_PADDING_Y, buffer);
        this->next->setParent(this);
        this->next->setCallback([]() {
            TriPlayer::next();
        });

        // Repeat icon
        buffer.resize(repeat_png_size);
        std::memcpy(&buffer[0], repeat_png, repeat_png_size);
        this->repeat = new Button(BUTTON_PADDING_X, BUTTON_PADDING_Y, buffer);
        this->repeat->setParent(this);
        buffer.resize(repeat_one_png_size);
        std::memcpy(&buffer[0], repeat_one_png, repeat_one_png_size);
        this->repeat->addAltImage(buffer);
        this->repeat->setCallback([this]() {
            if (this->repeatOn) {
                TriPlayer::setRepeatMode(this->repeatOne ? TriPlayer::Repeat::All : TriPlayer::Repeat::Off);
            } else {
                TriPlayer::setRepeatMode(TriPlayer::Repeat::One);
            }
        });

        // Stop button
        this->stop = new Button(BUTTON_PADDING_X * 1.5, BUTTON_PADDING_Y * 1.5, "Stop Sysmodule\n", STOP_FONT_SIZE);
        this->stop->setParent(this);
        this->stop->setCallback([]() {
            if (TriPlayer::stopSysmodule()) {
                tsl::goBack();
            }
        });

        // Initialize empty variables
        this->title = "Nothing playing!\n";
        this->artist = "Play a song\n";
        this->position = "0:00\n";
        this->duration = "0:00\n";
        this->durationSecs = 0;
        this->positionSecs = 0;
        this->playing = false;
        this->repeatOn = false;
        this->repeatOne = false;
        this->shuffled = false;
    }

    void Player::setTitle(const std::string & str) {
        this->title = str.substr(0, 20);
        if (this->title.length() == 20) {
            this->title += "...";
        }
        this->title += "\n";
        this->title.shrink_to_fit();
    }

    void Player::setArtist(const std::string & str) {
        this->artist = str.substr(0, 30);
        if (this->artist.length() == 30) {
            this->artist += "...";
        }
        this->artist += "\n";
        this->artist.shrink_to_fit();
    }

    void Player::setPosition(const double pos) {
        this->positionSecs = (pos/100.0) * this->durationSecs;
        this->position = Utils::secondsToHMS(this->positionSecs);
    }

    void Player::setDuration(const unsigned int sec) {
        this->durationSecs = sec;
        this->duration = Utils::secondsToHMS(sec) + "\n";
    }

    void Player::setPlaying(const bool playing) {
        this->playing = playing;
    }

    void Player::setRepeat(const bool on, const bool one) {
        this->repeatOn = on;
        this->repeatOne = one;
    }

    void Player::setShuffle(const bool shuffled) {
        this->shuffled = shuffled;
    }

    void Player::setAlbumArt(std::vector<uint8_t> & buf) {
        // Load default image if empty
        if (buf.empty()) {
            if (!this->defaultArt) {
                this->albumArt.pixels.clear();
                buf.resize(no_album_png_size);
                std::memcpy(&buf[0], no_album_png, no_album_png_size);
                this->defaultArt = true;

            } else {
                return;
            }

        } else {
            this->albumArt.pixels.clear();
            this->defaultArt = false;
        }

        // Otherwise extract as usual
        this->albumArt = Utils::convertPNGToBitmap(buf, this->getWidth() * 0.75, this->getWidth() * 0.75);
    }

    tsl::elm::Element * Player::requestFocus(tsl::elm::Element * old, tsl::FocusDirection dir) {
        // If "None" direction, highlight the play/pause button
        switch (dir) {
            case tsl::FocusDirection::None:
                return this->play->requestFocus(old, dir);

            // Otherwise if right/left attempt to move
            case tsl::FocusDirection::Left:
                if (old == this->repeat) {
                    return this->next->requestFocus(old, dir);

                } else if (old == this->next) {
                    return this->play->requestFocus(old, dir);

                } else if (old == this->play) {
                    return this->previous->requestFocus(old, dir);

                } else if (old == this->previous) {
                    return this->shuffle->requestFocus(old, dir);
                }
                break;

            // Otherwise if right/left attempt to move
            case tsl::FocusDirection::Right:
                if (old == this->next) {
                    return this->repeat->requestFocus(old, dir);

                } else if (old == this->play) {
                    return this->next->requestFocus(old, dir);

                } else if (old == this->previous) {
                    return this->play->requestFocus(old, dir);

                } else if (old == this->shuffle) {
                    return this->previous->requestFocus(old, dir);
                }
                break;

            // Move off stop button if highlighted
            case tsl::FocusDirection::Up:
                if (old == this->stop) {
                    return this->play->requestFocus(old, dir);
                }
                break;

            // Move to stop button if not highlighted
            case tsl::FocusDirection::Down:
                if (old != this->stop) {
                    return this->stop->requestFocus(old, dir);
                }
                break;
        }

        // If not handled return the currently focussed element
        return old->requestFocus(old, dir);
    }

    void Player::draw(tsl::gfx::Renderer * renderer) {
        std::pair<u32, u32> dimensions;
        u16 nextY = this->getY();

        // Now render album art
        if (!this->albumArt.pixels.empty()) {
            renderer->drawBitmap(this->getX() + (this->getWidth() - this->albumArt.width)/2, nextY, this->albumArt.width, this->albumArt.height, &this->albumArt.pixels[0]);
        }
        nextY += this->getWidth() * 0.875;

        // Song title (centered)
        dimensions = renderer->drawString(this->title.c_str(), false, 0, 0, TITLE_FONT_SIZE, 0x0);
        renderer->drawString(this->title.c_str(), false, this->getX() + (this->getWidth() - dimensions.first)/2, nextY, TITLE_FONT_SIZE, tsl::style::color::ColorText);
        nextY += dimensions.second * 1.3;

        // Song artist(s) (centered)
        dimensions = renderer->drawString(this->artist.c_str(), false, 0, 0, ARTIST_FONT_SIZE, 0x0);
        renderer->drawString(this->artist.c_str(), false, this->getX() + (this->getWidth() - dimensions.first)/2, nextY, ARTIST_FONT_SIZE, tsl::style::color::ColorDescription);
        nextY += dimensions.second * 2;

        // Position (get width to right align)
        dimensions = renderer->drawString(this->position.c_str(), false, 0, 0, TIME_FONT_SIZE, 0x0);
        renderer->drawString(this->position.c_str(), false, this->getX() + this->getWidth() * 0.15 - dimensions.first, nextY, TIME_FONT_SIZE, tsl::style::color::ColorText);

        // Duration
        renderer->drawString(this->duration.c_str(), false, this->getX() + this->getWidth() * 0.85, nextY, TIME_FONT_SIZE, tsl::style::color::ColorText);
        nextY -= 6;

        // Scroll bar
        renderer->drawRect(this->getX() + this->getWidth() * 0.2, nextY - 2, this->getWidth() * 0.6, 4, tsl::style::color::ColorHandle);
        renderer->drawRect(this->getX() + this->getWidth() * 0.2, nextY - 2, (this->getWidth() * 0.6) * (this->positionSecs/static_cast<double>(this->durationSecs)), 4, tsl::style::color::ColorHighlight);

        // Show play/pause button based on state
        this->play->showAltImage(this->playing);
        this->play->frame(renderer);

        // Show repeat icon and colour based on state
        this->repeat->setColour(this->repeatOn ? tsl::style::color::ColorHighlight : tsl::style::color::ColorHandle);
        this->repeat->showAltImage(this->repeatOne);
        this->repeat->frame(renderer);

        // Set shuffle colour based on if it's on or not
        this->shuffle->setColour(this->shuffled ? tsl::style::color::ColorHighlight : tsl::style::color::ColorHandle);
        this->shuffle->frame(renderer);

        // Simply draw other buttons
        this->previous->frame(renderer);
        this->next->frame(renderer);
        this->stop->frame(renderer);
    }

    void Player::layout(u16 parentX, u16 parentY, u16 parentW, u16 parentH) {
        // Be sneaky and force ourself up a few pixels
        this->setBoundaries(parentX + 35, parentY + 80, parentW - 85, parentH - 73 - 80);

        // Position buttons
        int nextY = this->getY() + this->getHeight() - 160;
        this->play->setBoundaries(this->getX() + (this->getWidth() - this->play->getWidth())/2, nextY, this->play->getWidth(), this->play->getHeight());

        nextY += 17;
        this->previous->setBoundaries(this->getX() + this->getWidth() * 0.22, nextY, this->previous->getWidth(), this->previous->getHeight());
        this->next->setBoundaries(this->getX() + this->getWidth() * 0.78 - this->next->getWidth(), nextY, this->next->getWidth(), this->next->getHeight());

        nextY += 2;
        this->shuffle->setBoundaries(this->getX() + 5, nextY, this->shuffle->getWidth(), this->shuffle->getHeight());
        this->repeat->setBoundaries(this->getX() + this->getWidth() - 10 - this->repeat->getWidth(), nextY, this->repeat->getWidth(), this->repeat->getHeight());

        nextY += 75;
        this->stop->setBoundaries(this->getX() + (this->getWidth() - 162)/2, nextY, this->stop->getWidth(), this->stop->getHeight());
    }

    bool Player::onTouch(tsl::elm::TouchEvent event, s32 currX, s32 currY, s32 prevX, s32 prevY, s32 initX, s32 initY) {
        if (event != tsl::elm::TouchEvent::Release) {
            return false;
        }

        // Check which element is touched and activate callback
        if (currX > this->shuffle->getX() && currX < this->shuffle->getX() + this->shuffle->getWidth() && currY > this->shuffle->getY() && currY < this->shuffle->getY() + this->shuffle->getHeight()) {
            this->shuffle->runCallback();
            return true;

        } else if (currX > this->previous->getX() && currX < this->previous->getX() + this->previous->getWidth() && currY > this->previous->getY() && currY < this->previous->getY() + this->previous->getHeight()) {
            this->previous->runCallback();
            return true;

        } else if (currX > this->play->getX() && currX < this->play->getX() + this->play->getWidth() && currY > this->play->getY() && currY < this->play->getY() + this->play->getHeight()) {
            this->play->runCallback();
            return true;

        } else if (currX > this->next->getX() && currX < this->next->getX() + this->next->getWidth() && currY > this->next->getY() && currY < this->next->getY() + this->next->getHeight()) {
            this->next->runCallback();
            return true;

        } else if (currX > this->repeat->getX() && currX < this->repeat->getX() + this->repeat->getWidth() && currY > this->repeat->getY() && currY < this->repeat->getY() + this->repeat->getHeight()) {
            this->repeat->runCallback();
            return true;

        } else if (currX > this->stop->getX() && currX < this->stop->getX() + this->stop->getWidth() && currY > this->stop->getY() && currY < this->stop->getY() + this->stop->getHeight()) {
            this->stop->runCallback();
            return true;
        }

        return false;
    }

    Player::~Player() {
        delete this->shuffle;
        delete this->previous;
        delete this->play;
        delete this->next;
        delete this->repeat;
        delete this->stop;
    }
};