#ifndef AETHER_HELPBUTTON_HPP
#define AETHER_HELPBUTTON_HPP

#include "primary/Text.hpp"

namespace Aether {
    // HelpButton is a round button displaying a question mark as seen in Horizon.
    class HelpButton : public Element {
        private:
            Text * text;

        public:
            // Creates the texture
            // X, Y, diameter, callback
            HelpButton(int, int, int, std::function<void()>);

            // Getter + setter for colours
            Colour getColour();
            void setColour(Colour c);

            // Highlight is round
            void renderHighlighted();
            void renderSelected();
    };
};

#endif