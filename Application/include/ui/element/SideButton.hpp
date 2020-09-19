#ifndef ELEMENT_SIDEBUTTON_HPP
#define ELEMENT_SIDEBUTTON_HPP

#include "Aether/Aether.hpp"

namespace CustomElm {
    // A SideButton appears in the left hand side pane
    class SideButton : public Aether::Element {
        private:
            // Rectangle to show if selected
            Aether::Rectangle * rect;
            bool isActive;

            Aether::Image * icon;
            Aether::Text * text;

            // Colours
            Aether::Colour inactive;
            Aether::Colour active;

            void positionElements();

        public:
            // X, Y, W
            SideButton(int, int, int);

            // Set colours
            void setInactiveColour(Aether::Colour);
            void setActiveColour(Aether::Colour);

            // Set icon (pass image pointer) and text
            void setIcon(Aether::Image *);
            void setText(std::string);

            // Set (in)active
            void setActivated(bool);

            void setW(int);
    };
};

#endif