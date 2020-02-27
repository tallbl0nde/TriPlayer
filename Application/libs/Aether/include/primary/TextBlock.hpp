#ifndef AETHER_TEXTBLOCK_HPP
#define AETHER_TEXTBLOCK_HPP

#include "base/BaseText.hpp"

namespace Aether {
    // TextBlock extends BaseText by wrapping text with the given width
    // It's literally a block of text
    class TextBlock : public BaseText {
        private:
            // Width in pixels to wrap at
            unsigned int wrapWidth_;

            void redrawTexture();

        public:
            // Constructor requires: string, font size, wrap width (px) + optional FontType
            TextBlock(int, int, std::string, unsigned int, unsigned int, FontType = FontType::Normal, FontStyle = FontStyle::Regular);

            // Getter + setter for wrapWidth
            // Altering requires re-render of text
            unsigned int wrapWidth();
            void setWrapWidth(unsigned int);
    };
};

#endif