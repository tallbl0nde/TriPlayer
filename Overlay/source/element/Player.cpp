#include <cstring>
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
        this->shuffleIcon = Utils::convertPNGToBitmap(buffer);

        // Previous icon
        buffer.resize(previous_png_size);
        std::memcpy(&buffer[0], previous_png, previous_png_size);
        this->previousIcon = Utils::convertPNGToBitmap(buffer);

        // Play icon
        buffer.resize(play_png_size);
        std::memcpy(&buffer[0], play_png, play_png_size);
        this->playIcon = Utils::convertPNGToBitmap(buffer);

        // Pause icon
        buffer.resize(pause_png_size);
        std::memcpy(&buffer[0], pause_png, pause_png_size);
        this->pauseIcon = Utils::convertPNGToBitmap(buffer);

        // Next icon
        buffer.resize(next_png_size);
        std::memcpy(&buffer[0], next_png, next_png_size);
        this->nextIcon = Utils::convertPNGToBitmap(buffer);

        // Repeat icon
        buffer.resize(repeat_png_size);
        std::memcpy(&buffer[0], repeat_png, repeat_png_size);
        this->repeatIcon = Utils::convertPNGToBitmap(buffer);

        // Repeat one icon
        buffer.resize(repeat_one_png_size);
        std::memcpy(&buffer[0], repeat_one_png, repeat_one_png_size);
        this->repeatOneIcon = Utils::convertPNGToBitmap(buffer);
    }

    tsl::elm::Element * Player::requestFocus(tsl::elm::Element * old, tsl::FocusDirection direction) {
        return nullptr;
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
        nextY += 40;

        // Play/pause
        if (!this->playIcon.pixels.empty()) {
            renderer->drawBitmap(this->getX() + this->getWidth() * 0.5 - this->playIcon.width/2, nextY, this->playIcon.width, this->playIcon.height, &this->playIcon.pixels[0]);
        }
        nextY += 17;

        // Previous
        if (!this->previousIcon.pixels.empty()) {
            renderer->drawBitmap(this->getX() + this->getWidth() * 0.3 - 24, nextY, this->previousIcon.width, this->previousIcon.height, &this->previousIcon.pixels[0]);
        }

        // Next
        if (!this->nextIcon.pixels.empty()) {
            renderer->drawBitmap(this->getX() + this->getWidth() * 0.7, nextY, this->nextIcon.width, this->nextIcon.height, &this->nextIcon.pixels[0]);
        }
        nextY += 2;

        // Shuffle
        if (!this->shuffleIcon.pixels.empty()) {
            renderer->drawBitmap(this->getX() + 5, nextY, this->shuffleIcon.width, this->shuffleIcon.height, &this->shuffleIcon.pixels[0]);
        }

        // Repeat
        if (!this->repeatIcon.pixels.empty()) {
            renderer->drawBitmap(this->getX() + this->getWidth() - this->repeatIcon.width - 10, nextY, this->repeatIcon.width, this->repeatIcon.height, &this->repeatIcon.pixels[0]);
        }
        nextY += 70;

        // Repeat one

        // Stop button
        renderer->drawRect(this->getX() + this->getWidth() * 0.5 - 75, nextY, 150, 40, 0xeeee);
    }

    void Player::layout(u16 parentX, u16 parentY, u16 parentW, u16 parentH) {
        // Be sneaky and force ourself up a few pixels
        this->setBoundaries(parentX + 35, parentY + 80, parentW - 85, parentH - 73 - 80);

        // Resize the album art (does nothing if already the same size)
        if (!Utils::resizeBitmap(this->albumArt, this->getWidth() * 0.75, this->getWidth() * 0.75)) {
            this->albumArt.pixels.clear();
        }
    }

    Player::~Player() {

    }
};