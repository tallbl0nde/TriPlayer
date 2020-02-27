#include "ThreadedText.hpp"

namespace Aether {
    ThreadedText::ThreadedText(int x, int y, std::string s, unsigned int f, FontType t, FontStyle l) : Text(x, y, s, f, t, l), Threaded( [this]() {this->createSurface();} ) {
        this->surface = nullptr;
    }

    void ThreadedText::redrawTexture() {
        this->redraw = true;
    }

    void ThreadedText::createSurface() {
        // this->setScroll(this->scroll());
        // int style;
        // switch (this->fontStyle) {
        //     case FontStyle::Regular:
        //         style = TTF_STYLE_NORMAL;
        //         break;

        //     case FontStyle::Bold:
        //         style = TTF_STYLE_BOLD;
        //         break;

        //     case FontStyle::Italic:
        //         style = TTF_STYLE_ITALIC;
        //         break;

        //     case FontStyle::Underline:
        //         style = TTF_STYLE_UNDERLINE;
        //         break;

        //     case FontStyle::Strikethrough:
        //         style = TTF_STYLE_STRIKETHROUGH;
        //         break;
        // }
        // this->surface = SDLHelper::renderTextSurface(this->string_.c_str(), this->fontSize_, style, (this->fontType == FontType::Extended) ? true : false);
    }

    void ThreadedText::update(uint32_t dt) {
        // Draw surface if needed
        if (this->redraw) {
            this->start();
            this->redraw = false;
        }

        // Convert surface to texture
        // if (this->status() == ThreadStatus::Finished && this->surface != nullptr) {
        //     this->setTexture(SDLHelper::surfaceToTexture(this->surface));
        //     SDLHelper::destroySurface(this->surface);
        //     this->surface = nullptr;
        // }

        // Text::update(dt);
    }
};