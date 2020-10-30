#include "element/Player.hpp"

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

    }

    tsl::elm::Element * Player::requestFocus(tsl::elm::Element * old, tsl::FocusDirection direction) {
        return nullptr;
    }

    void Player::draw(tsl::gfx::Renderer * renderer) {
        std::pair<u32, u32> dimensions;
        u16 nextY = this->getY();

        // Album art
        renderer->drawRect(this->getX() + this->getWidth() * 0.125, nextY, this->getWidth() * 0.75, this->getWidth() * 0.75, 0xeeee);
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
        renderer->drawCircle(this->getX() + this->getWidth() * 0.5, nextY + 30, 30, true, 0xffff);
        nextY += 17;

        // Previous
        renderer->drawRect(this->getX() + this->getWidth() * 0.3 - 24, nextY, 24, 26, 0xffff);

        // Next
        renderer->drawRect(this->getX() + this->getWidth() * 0.7, nextY, 24, 26, 0xffff);
        nextY += 2;

        // Shuffle
        renderer->drawRect(this->getX() + 5, nextY, 30, 21, 0xffff);

        // Repeat
        renderer->drawRect(this->getX() + this->getWidth() - 35, nextY, 30, 21, 0xffff);
        nextY += 70;

        // Stop button
        renderer->drawRect(this->getX() + this->getWidth() * 0.5 - 75, nextY, 150, 40, 0xeeee);
    }

    void Player::layout(u16 parentX, u16 parentY, u16 parentW, u16 parentH) {
        // Be sneaky and force ourself up a few pixels
        this->setBoundaries(parentX + 35, parentY + 80, parentW - 85, parentH - 73 - 80);
    }

    Player::~Player() {

    }
};