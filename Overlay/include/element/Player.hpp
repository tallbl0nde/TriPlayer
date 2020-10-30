#ifndef ELEMENT_PLAYER_HPP
#define ELEMENT_PLAYER_HPP

#include "tesla.hpp"

namespace Element {
    class Player : public tsl::elm::Element {
        private:

        public:
            Player();

            tsl::elm::Element * requestFocus(tsl::elm::Element *, tsl::FocusDirection);

            void draw(tsl::gfx::Renderer *);

            void layout(u16, u16, u16, u16);

            ~Player();
    };
};

#endif