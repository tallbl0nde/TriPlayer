#ifndef ELEMENT_SLIDER_HPP
#define ELEMENT_SLIDER_HPP

#include "Aether.hpp"

namespace CustomElm {
    // A slider is a bar with a knob that can be changed by touch/controller
    // NOTE: Ellipse position is hardcoded because the SDL library can't render it in an accurate position :/
    // NOTE 2.0: Callback is called on release!
    // I should redo this at some point to make it more flexible!!
    class Slider : public Aether::Element {
        private:
            // Use RoundProgressBar as coloured bar
            Aether::RoundProgressBar * bar;
            // Knob element
            Aether::Ellipse * knob;

            // Amount to move when button pressed
            float nudge;

        public:
            // X, Y, W (rightmost point when set 100 - leftmost point when set zero), H (knob diameter), height of bar
            Slider(int, int, int, int, int);

            bool handleEvent(Aether::InputEvent *);
            void render();

            float value();
            void setValue(float);

            // Set amount to inc/dec on button press
            void setNudge(float);

            // Set colours
            void setBarBackgroundColour(Aether::Colour);
            void setBarForegroundColour(Aether::Colour);
            void setKnobColour(Aether::Colour);

            // Elements must be repositioned when dimensions are changed
            void setW(int);
            void setH(int);
            void setBarH(int);

            // We only highlight around the knob!
            void renderHighlighted();
            void renderSelected();
    };
};

#endif