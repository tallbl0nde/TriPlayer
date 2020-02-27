#ifndef AETHER_SPINNER_HPP
#define AETHER_SPINNER_HPP

#include "base/Container.hpp"
#include "primary/Text.hpp"
#include "utils/Types.hpp"

namespace Aether {
    // A Spinner contains a number which can be increased/decreased by pressing
    // the associated up/down buttons or the dpad.
    // Note that this element has a default height and should not be changed!!
    class Spinner : public Container {
        private:
            // Up arrow, down arrow + actual value/text
            Text * up;
            Text * down;
            Text * str;
            // Label string (hidden if not set)
            Text * label_;

            // Set true when spinner is focussed (needed for colours)
            bool isFocussed;

            // Set true to allow values to wrap from max - min and vice versa
            bool wrap;
            // Number of zeroes to pad number with
            unsigned int padding;
            // Amount to inc/dec
            int amount;
            // Should be obvious
            int min_;
            int max_;
            int value_;
            void incrementVal();
            void decrementVal();

            // Colours
            Colour arrowC;
            Colour highlightC;
            Colour textC;

        public:
            // Constructor initializes value to 0
            // x, y, w (optional)
            Spinner(int, int, int = 90);

            // Need to handle up/down button presses
            bool handleEvent(InputEvent *);
            // Set colours if focussed or not
            void update(uint32_t);
            // Sets isFocussed bool
            void setActive();
            void setInactive();

            // Get + set whether to wrap values around
            bool wrapAround();
            void setWrapAround(bool);

            // Get + set number of digits (padded with zeroes)
            unsigned int digits();
            // Set to zero to disable padding
            void setDigits(unsigned int);

            // Set a label for the spinner (give empty string to remove)
            // Will increase (or on removal  decrease) height!
            void setLabel(std::string);
            // Return currently set label
            std::string label();

            // Get + set inc/dec amount
            int changeAmount();
            void setChangeAmount(int);

            // Get + set value
            int value();
            void setValue(int);

            // Get + set allowed range
            int min();
            void setMin(int);
            int max();
            void setMax(int);

            // Getter + setters for colours (arrow also sets label colour)
            Colour getArrowColour();
            void setArrowColour(Colour);
            Colour getHighlightColour();
            void setHighlightColour(Colour);
            Colour getTextColour();
            void setTextColour(Colour);
    };
};

#endif