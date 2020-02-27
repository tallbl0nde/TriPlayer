#ifndef AETHER_DATETIME_HPP
#define AETHER_DATETIME_HPP

#include <ctime>
#include "horizon/button/BorderButton.hpp"
#include "horizon/controls/Controls.hpp"
#include "horizon/input/Spinner.hpp"
#include "primary/Rectangle.hpp"
#include "Overlay.hpp"

namespace Aether {
    // Spinners to show (create custom by passing bitwise OR of multiple flags!)
    // Each bit represents in the order: -- D/M/Y H:M:S
    enum class DTFlag {
        DateTime = 0b00111111,
        Date = 0b00111000,
        Time = 0b00000111,
        Day = 0b00100000,
        Month = 0b00010000,
        Year = 0b00001000,
        Hour = 0b00000100,
        Min = 0b00000010,
        Sex = 0b00000001
    };

    // Bitwise OR operator for DTFlag
    inline DTFlag operator|(DTFlag a, DTFlag b) {
        return static_cast<DTFlag>(static_cast<int>(a) | static_cast<int>(b));
    }

    // Bitwise AND operator for DTFlag
    inline bool operator&(DTFlag a, DTFlag b) {
        return static_cast<int>(a) & static_cast<int>(b);
    }

    // The DateTime overlay is used to get the user to select a date/time.
    // It must be passed a tm struct on creation which has it's value changed
    // in place when closed (not updated when closed by pressing B!)
    // ** Expect a segfault if the tm struct is deleted by the time this overlay is closed! **
    class DateTime : public Overlay {
        private:
            // Reference to passed tm
            struct tm & refTm;

            // Pointers to elements
            Controls * ctrl;
            Rectangle * rect, * top, * bottom;
            Text * title;
            BorderButton * button;

            // Keep pointers to each item
            Spinner * day;
            Spinner * month;
            Spinner * year;
            Spinner * hour;
            Spinner * min;
            Spinner * sec;

            // Separating characters
            Text * div1;
            Text * div2;
            Text * col1;
            Text * col2;

            // Updates value in tm
            void setTmValues();

        public:
            // Pass title, tm struct and type of picker (leave empty for complete date and time)
            DateTime(std::string, struct tm &, DTFlag = DTFlag::DateTime);

            // Whenever an event is handled update max days
            bool handleEvent(InputEvent *);

            // Getters + setters colours
            Colour getBackgroundColour();
            void setBackgroundColour(Colour);
            Colour getHighlightColour();
            void setHighlightColour(Colour);
            Colour getInactiveColour();
            void setInactiveColour(Colour);
            Colour getSeparatorColour();
            void setSeparatorColour(Colour);
            Colour getTextColour();
            void setTextColour(Colour);

            // Set all colours
            // BG, Highlighted, Inactive, Separator, Text
            void setAllColours(Colour, Colour, Colour, Colour, Colour);
    };
};

#endif