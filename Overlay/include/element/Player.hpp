#ifndef ELEMENT_PLAYER_HPP
#define ELEMENT_PLAYER_HPP

#include "tesla.hpp"
#include "Utils.hpp"

namespace Element {
    class Player : public tsl::elm::Element {
        private:
            Bitmap albumArt;
            Bitmap shuffleIcon;
            Bitmap previousIcon;
            Bitmap playIcon;
            Bitmap pauseIcon;
            Bitmap nextIcon;
            Bitmap repeatIcon;
            Bitmap repeatOneIcon;

        public:
            Player();

            tsl::elm::Element * requestFocus(tsl::elm::Element *, tsl::FocusDirection);

            void draw(tsl::gfx::Renderer *);

            void layout(u16, u16, u16, u16);

            ~Player();
    };
};

#endif