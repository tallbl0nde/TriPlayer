#ifndef ELEMENT_MENUBUTTON_HPP
#define ELEMENT_MENUBUTTON_HPP

#include "Aether/Aether.hpp"

namespace CustomElm {
    // A MenuButton appears in SongMenu
    class MenuButton : public Aether::Element {
        private:
            // Elements
            Aether::Image * icon;
            Aether::Text * text;

        public:
            // X, Y, W, H
            MenuButton(int, int, int, int);

            // Set colours
            void setIconColour(Aether::Colour);
            void setTextColour(Aether::Colour);

            // Set icon (pass image pointer) and text
            void setIcon(Aether::Image *);
            void setText(std::string);
    };
};

#endif