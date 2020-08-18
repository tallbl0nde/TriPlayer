#ifndef ELEMENT_LISTHEADINGCOUNT_HPP
#define ELEMENT_LISTHEADINGCOUNT_HPP

#include "Aether/Aether.hpp"

namespace CustomElm {
    // Heading element in a list used to indicate how many items are in the section below
    class ListHeadingCount : public Aether::Element {
        private:
            // Strings
            Aether::Text * heading;
            Aether::Text * count;

        public:
            // Create blank elements
            ListHeadingCount();

            // Set strings
            void setHeadingString(const std::string &);
            void setCount(const size_t);
            void setTextColour(const Aether::Colour &);
    };
};

#endif