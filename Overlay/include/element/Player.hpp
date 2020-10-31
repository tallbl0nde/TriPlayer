#ifndef ELEMENT_PLAYER_HPP
#define ELEMENT_PLAYER_HPP

#include "tesla.hpp"
#include "Utils.hpp"

namespace Element {
    // Forward declaration
    class Button;

    // The Player element contains all the text, controls and other
    // elements contained in the overlay.
    class Player : public tsl::elm::Element {
        private:
            // Player elements
            Bitmap albumArt;        // Album art image
            Button * shuffle;       // Shuffle button
            Button * previous;      // Previous button
            Button * play;          // Play/pause button
            Button * next;          // Next button
            Button * repeat;        // Repeat button

            // Song metadata
            bool defaultArt;        // albumArt contains default image
            std::string title;      // Song title
            std::string artist;     // Artist title
            std::string position;   // Position text
            std::string duration;   // Duration text

            // Cached values
            unsigned int durationSecs;
            unsigned int positionSecs;
            bool playing;
            bool repeatOn;
            bool repeatOne;
            bool shuffled;

        public:
            // Constructor sets up all elements
            Player();

            // Functions passed values to set the state of the player + controls
            void setTitle(const std::string &);
            void setArtist(const std::string &);
            void setPosition(const double);
            void setDuration(const unsigned int);
            void setPlaying(const bool);
            void setRepeat(const bool, const bool);
            void setShuffle(const bool);
            void setAlbumArt(std::vector<uint8_t> &);

            // Override to call appropriate child requestFocus()
            tsl::elm::Element * requestFocus(tsl::elm::Element *, tsl::FocusDirection);

            // Draw all contained buttons + text
            void draw(tsl::gfx::Renderer *);

            // Initially position all elements
            void layout(u16, u16, u16, u16);

            // Check if within buttons and call callback
            bool onTouch(tsl::elm::TouchEvent, s32, s32, s32, s32, s32, s32);

            // Delete all button elements
            ~Player();
    };
};

#endif