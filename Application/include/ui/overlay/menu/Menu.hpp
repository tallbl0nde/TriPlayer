#ifndef OVERLAY_MENU_MENU_HPP
#define OVERLAY_MENU_MENU_HPP

#include "Aether/Aether.hpp"

namespace CustomOvl::Menu {
    // Implements the basic things that all menus have in common
    class Menu : public Aether::Overlay {
        protected:
            // Rectangle 'background'
            Aether::Rectangle * bg;
            // Line texture used to separate groups
            SDL_Texture * line;
            Aether::Colour lineColour;

        public:
            // Constructor initializes above elements
            Menu();

            // Set colours
            void setBackgroundColour(Aether::Colour);
            void setLineColour(Aether::Colour);

            // Close menu if tapped outside of rectangle
            bool handleEvent(Aether::InputEvent *);
            // Children will need to implement render() to handle line

            // Delete line texture
            ~Menu();
    };
};

#endif