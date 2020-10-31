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

        public:
            Player();

            tsl::elm::Element * requestFocus(tsl::elm::Element *, tsl::FocusDirection);

            void draw(tsl::gfx::Renderer *);

            void layout(u16, u16, u16, u16);

            ~Player();
    };
};

#endif