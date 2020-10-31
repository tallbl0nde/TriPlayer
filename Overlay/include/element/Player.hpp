#ifndef ELEMENT_PLAYER_HPP
#define ELEMENT_PLAYER_HPP

#include "tesla.hpp"
#include "Utils.hpp"

namespace Element {
    // Forward declaration
    class Button;

    // The Player element...
    class Player : public tsl::elm::Element {
        private:
            Bitmap albumArt;
            Button * shuffle;
            Button * previous;
            Button * play;
            Button * next;
            Button * repeat;

            // Song metadata
            bool defaultArt;
            std::string title;
            std::string artist;
            std::string position;
            std::string duration;

            unsigned int durationSecs;
            unsigned int positionSecs;

            bool playing;
            bool repeatOn;
            bool repeatOne;
            bool shuffled;

        public:
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

            tsl::elm::Element * requestFocus(tsl::elm::Element *, tsl::FocusDirection);

            void draw(tsl::gfx::Renderer *);

            void layout(u16, u16, u16, u16);

            ~Player();
    };
};

#endif