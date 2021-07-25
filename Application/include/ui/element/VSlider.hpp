#ifndef ELEMENT_VSLIDER_HPP
#define ELEMENT_VSLIDER_HPP

#include "Aether/Aether.hpp"

namespace CustomElm {
    class VSlider : public Aether::Element {
        private:
            Aether::Rectangle * barBg;      // Background bar
            Aether::Rectangle * barFg;      // Forground bar
            Aether::Ellipse * knob;         // Knob
            float nudge;                    // Amount to move when button pressed
            float value_;                   // Value/position

        public:
            // X, Y, W, H, knob diameter
            VSlider(int, int, int, int, int);

            // Override to handle button events
            bool handleEvent(Aether::InputEvent *);
            void render();

            // Set/get value (0.0 - 100.0)
            float value();
            void setValue(float);

            // Set amount to inc/dec on button press
            void setNudge(float);

            // Set colours
            void setBarBackgroundColour(const Aether::Colour &);
            void setBarForegroundColour(const Aether::Colour &);
            void setKnobColour(const Aether::Colour &);

            // Override to only highlight the knob
            Aether::Drawable * renderHighlightBG();
            Aether::Drawable * renderHighlight();
            Aether::Drawable * renderSelection();
    };
};

#endif
