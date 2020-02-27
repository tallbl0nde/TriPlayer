#ifndef AETHER_TEXT_HPP
#define AETHER_TEXT_HPP

#include "base/BaseText.hpp"

namespace Aether {
    // Text extends BaseText by implementing scrolling when the text
    // overflows. Thus it's for single-line text.
    class Text : public BaseText {
        private:
            // Scroll if the texture width is greater than the specified width
            bool scroll_;
            // Pixels to scroll per second
            int scrollSpeed_;
            // Time since scroll finished (in ms) (only used internally)
            int scrollPauseTime;

            // Redraw the texture whenever relevant variables are changed
            void redrawTexture();

        public:
            // Constructor accepts string, font size and font type
            Text(int, int, std::string, unsigned int, FontType = FontType::Normal, FontStyle = FontStyle::Regular);

            // Getter + setter for scroll + scrollSpeed
            bool scroll();
            void setScroll(bool);
            int scrollSpeed();
            void setScrollSpeed(int);

            // Wrapper for BaseText functions that also renders the texture
            void setFontSize(unsigned int);
            void setString(std::string);

            // Update handles animating the scroll if necessary
            void update(uint32_t);

            // Adjusting width may need to adjust amount of text shown
            void setW(int);
    };
};

#endif