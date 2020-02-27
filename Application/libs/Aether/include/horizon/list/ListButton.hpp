#ifndef AETHER_LISTBUTTON_HPP
#define AETHER_LISTBUTTON_HPP

#include "primary/Rectangle.hpp"
#include "primary/Text.hpp"

namespace Aether {
    // A ListButton is similar to what appears in Horizon's
    // lists. It has a string and should be used to open something else.
    class ListButton : public Element {
        private:
            // Elements for lines + strings
            Rectangle * topR;
            Rectangle * bottomR;
            Text * text_;

        public:
            // Constructor takes string + function
            ListButton(std::string, std::function<void()>);

            // Getters + setters for colours
            Colour getLineColour();
            void setLineColour(Colour);
            Colour getTextColour();
            void setTextColour(Colour);
            // Set all colours at once: line, hint
            void setColours(Colour, Colour);

            // Getter + setter for string
            std::string text();
            void setText(std::string);
            unsigned int fontSize();
            void setFontSize(unsigned int);

            // Adjusting width requires changing width of rects
            void setW(int);
            // Adjusting height also moves same elements
            void setH(int);
    };
};

#endif