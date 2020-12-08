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

            // Position children
            void positionItems();

        public:
            MenuButton();

            // Scroll text when highlighted
            void update(uint32_t);

            // Set colours
            void setIconColour(Aether::Colour);
            void setTextColour(Aether::Colour);

            // Set icon (pass image pointer) and text
            void setIcon(Aether::Image *);
            void setText(std::string);

            // Changing the width needs the elements to be repositioned
            void setW(int);
    };
};

#endif