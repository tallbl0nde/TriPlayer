#ifndef ELEMENT_BUTTON_HPP
#define ELEMENT_BUTTON_HPP

#include "tesla.hpp"

namespace Element {
    class Button : public tsl::elm::Element {
        private:

        public:
            Button(u16, u16, u16, u16);

            void draw(tsl::gfx::Renderer *);

            void layout(u16, u16, u16, u16);
    };
};

#endif