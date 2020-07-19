#ifndef OVERLAY_MENU_MENU_HPP
#define OVERLAY_MENU_MENU_HPP

#include "Aether/Aether.hpp"

namespace CustomOvl::Menu {
    // Some menus allow to hide the top section
    enum class Type {
        Normal,     // Show the top section (image + name)
        HideTop     // Hide it
    };

    // Implements the basic things that all menus have in common
    class Menu : public Aether::Overlay {
        protected:
            // Rectangle 'background'
            Aether::Rectangle * bg;
            // Line texture used to separate groups
            SDL_Texture * line;
            Aether::Colour lineColour;
            // Type (used to prevent adding images, etc if not set)
            Type type;

        public:
            // Constructor initializes above elements
            Menu(Type);

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