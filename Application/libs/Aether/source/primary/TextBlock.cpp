#include "TextBlock.hpp"

namespace Aether {
    TextBlock::TextBlock(int x, int y, std::string s, unsigned int f, unsigned int w, FontType t, FontStyle l) : BaseText(x, y, s, f, t, l) {
        this->setWrapWidth(w);
    }

    void TextBlock::redrawTexture() {
        int style;
        switch (this->fontStyle) {
            case FontStyle::Regular:
                style = TTF_STYLE_NORMAL;
                break;

            case FontStyle::Bold:
                style = TTF_STYLE_BOLD;
                break;

            case FontStyle::Italic:
                style = TTF_STYLE_ITALIC;
                break;

            case FontStyle::Underline:
                style = TTF_STYLE_UNDERLINE;
                break;

            case FontStyle::Strikethrough:
                style = TTF_STYLE_STRIKETHROUGH;
                break;
        }
        this->setTexture(SDLHelper::renderTextWrapped(this->string_.c_str(), this->fontSize_, this->wrapWidth_, style, (this->fontType == FontType::Extended) ? true : false));
    }

    unsigned int TextBlock::wrapWidth() {
        return this->wrapWidth_;
    }

    void TextBlock::setWrapWidth(unsigned int w) {
        this->wrapWidth_ = w;
        this->redrawTexture();
    }
};