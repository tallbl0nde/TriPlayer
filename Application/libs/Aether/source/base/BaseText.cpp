#include "BaseText.hpp"

namespace Aether {
    BaseText::BaseText(int x, int y, std::string s, unsigned int f, FontType t, FontStyle l) : Texture(x, y) {
        this->fontSize_ = f;
        this->fontStyle = l;
        this->fontType = t;
        this->string_ = s;
    }

    std::string BaseText::string() {
        return this->string_;
    }

    void BaseText::setString(std::string s) {
        this->string_ = s;
    }

    unsigned int BaseText::fontSize() {
        return this->fontSize_;
    }

    void BaseText::setFontSize(unsigned int s) {
        this->fontSize_ = s;
    }
};