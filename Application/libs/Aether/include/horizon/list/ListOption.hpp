#ifndef AETHER_LISTOPTION_HPP
#define AETHER_LISTOPTION_HPP

#include "primary/Rectangle.hpp"
#include "primary/Text.hpp"

namespace Aether {
    // A ListOption is similar to what appears in Horizon's
    // lists. It has a string and value, as well as a callback function
    // which is called when selected.
    class ListOption : public Element {
        private:
            // Elements for lines + strings
            Rectangle * topR;
            Rectangle * bottomR;
            Text * hint_;
            Text * value_;

            // Reposition elements
            void positionElements();

        public:
            // Constructor takes strings + function
            ListOption(std::string, std::string, std::function<void()>);

            // Getters + setters for colours
            Colour getLineColour();
            void setLineColour(Colour);
            Colour getHintColour();
            void setHintColour(Colour);
            Colour getValueColour();
            void setValueColour(Colour);
            // Set all colours at once: line, hint, value
            void setColours(Colour, Colour, Colour);

            // Getter + setter for strings
            std::string hint();
            void setHint(std::string);
            std::string value();
            void setValue(std::string);

            // Adjust font size (also centers)
            void setFontSize(unsigned int);
            unsigned int fontSize();

            // Adjusting width requires changing width of rectangles + repositioning of text
            void setW(int);
            // Adjusting height moves elements
            void setH(int);
    };
};

#endif