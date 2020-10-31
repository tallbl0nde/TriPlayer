#include <cstring>
#include "element/Button.hpp"
#include "element/Player.hpp"

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

// Text colours
#define ARTIST_FONT_COLOUR 0xbbbb
#define TIME_FONT_COLOUR 0xffff
#define TITLE_FONT_COLOUR 0xffff

// Font sizes for text
#define ARTIST_FONT_SIZE 20
#define TIME_FONT_SIZE 20
#define TITLE_FONT_SIZE 24

namespace Element {
    Player::Player() : tsl::elm::Element() {
        // Read images into memory
        std::vector<uint8_t> buffer;

        // Default album art
        buffer.resize(no_album_png_size);
        std::memcpy(&buffer[0], no_album_png, no_album_png_size);
        this->albumArt = Utils::convertPNGToBitmap(buffer);

        // Shuffle icon
        buffer.resize(shuffle_png_size);
        std::memcpy(&buffer[0], shuffle_png, shuffle_png_size);
        this->shuffle = new Button(BUTTON_PADDING_X, BUTTON_PADDING_Y, buffer);
        this->shuffle->setParent(this);

        // Previous icon
        buffer.resize(previous_png_size);
        std::memcpy(&buffer[0], previous_png, previous_png_size);
        this->previous = new Button(BUTTON_PADDING_X, BUTTON_PADDING_Y, buffer);
        this->previous->setParent(this);

        // Play/pause icon
        buffer.resize(play_png_size);
        std::memcpy(&buffer[0], play_png, play_png_size);
        this->play = new Button(BUTTON_PADDING_X, BUTTON_PADDING_Y, buffer);
        this->play->setParent(this);
        buffer.resize(pause_png_size);
        std::memcpy(&buffer[0], pause_png, pause_png_size);
        this->play->addAltImage(buffer);

        // Next icon
        buffer.resize(next_png_size);
        std::memcpy(&buffer[0], next_png, next_png_size);
        this->next = new Button(BUTTON_PADDING_X, BUTTON_PADDING_Y, buffer);
        this->next->setParent(this);

        // Repeat icon
        buffer.resize(repeat_png_size);
        std::memcpy(&buffer[0], repeat_png, repeat_png_size);
        this->repeat = new Button(BUTTON_PADDING_X, BUTTON_PADDING_Y, buffer);
        this->repeat->setParent(this);
        buffer.resize(repeat_one_png_size);
        std::memcpy(&buffer[0], repeat_one_png, repeat_one_png_size);
        this->repeat->addAltImage(buffer);
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
                // if (old == stopButton) {
                    // return this->play->requestFocus(old, dir);
                // }
                break;

            // Move to stop button if not highlighted
            case tsl::FocusDirection::Down:
                // if (old != stopButton) {
                    // return this->stopButton->requestFocus(old, dir);
                // }
                break;
        }

        // If not handled return the currently focussed element
        return old->requestFocus(old, dir);
    }

    void Player::draw(tsl::gfx::Renderer * renderer) {
        std::pair<u32, u32> dimensions;
        u16 nextY = this->getY();

        // Album art
        if (!this->albumArt.pixels.empty()) {
            renderer->drawBitmap(this->getX() + (this->getWidth() - this->albumArt.width)/2, nextY, this->albumArt.width, this->albumArt.height, &this->albumArt.pixels[0]);
        }
        nextY += this->getWidth() * 0.875;

        // Song title (centered)
        dimensions = renderer->drawString("Song Title\n", false, 0, 0, TITLE_FONT_SIZE, 0x0);
        renderer->drawString("Song Title", false, this->getX() + (this->getWidth() - dimensions.first)/2, nextY, TITLE_FONT_SIZE, TITLE_FONT_COLOUR);
        nextY += dimensions.second * 1.3;

        // Song artist(s) (centered)
        dimensions = renderer->drawString("Song Artist(s)\n", false, 0, 0, ARTIST_FONT_SIZE, 0x0);
        renderer->drawString("Song Artist(s)", false, this->getX() + (this->getWidth() - dimensions.first)/2, nextY, ARTIST_FONT_SIZE, ARTIST_FONT_COLOUR);
        nextY += dimensions.second * 1.8;

        // Position (get width to right align)
        dimensions = renderer->drawString("0:00:00\n", false, 0, 0, TIME_FONT_SIZE, 0x0);
        renderer->drawString("0:00:00", false, this->getX() + this->getWidth() * 0.15 - dimensions.first, nextY, TIME_FONT_SIZE, TIME_FONT_COLOUR);

        // Duration
        renderer->drawString("0:00:00", false, this->getX() + this->getWidth() * 0.85, nextY, TIME_FONT_SIZE, TIME_FONT_COLOUR);
        nextY -= 6;

        // Scroll bar
        renderer->drawRect(this->getX() + this->getWidth() * 0.2, nextY - 2, this->getWidth() * 0.6, 4, 0xffff);
        nextY += 125;

        // Draw buttons
        this->shuffle->frame(renderer);
        this->previous->frame(renderer);
        this->play->frame(renderer);
        this->next->frame(renderer);
        this->repeat->frame(renderer);

        // Stop button
        renderer->drawRect(this->getX() + this->getWidth() * 0.5 - 75, nextY, 150, 40, 0xeeee);
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

        // Resize the album art (does nothing if already the same size)
        if (!Utils::resizeBitmap(this->albumArt, this->getWidth() * 0.75, this->getWidth() * 0.75)) {
            this->albumArt.pixels.clear();
        }
    }

    Player::~Player() {
        delete this->shuffle;
        delete this->previous;
        delete this->play;
        delete this->next;
        delete this->repeat;
    }
};