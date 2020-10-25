#include "ui/element/NumberBox.hpp"

// Box corner radius
#define BOX_CORNER_RAD 10
// Highlight corner radius
#define HI_CORNER_RAD 16
// Padding around text
#define PADDING 13

namespace CustomElm {
    NumberBox::NumberBox(int x, int y, int w, int h) : Element(x, y, w, h) {
        this->rect = new Aether::Rectangle(this->x(), this->y(), this->w(), this->h(), BOX_CORNER_RAD);
        this->addElement(this->rect);
        this->text = new Aether::Text(this->x() + PADDING, 0, "", this->h() - 2*PADDING);
        this->text->setY(this->y() + (this->h() - this->text->h())/2);
        this->addElement(this->text);

        // Set initial numpad config
        this->conf.value = 0;
        this->conf.maxDigits = 10;
        this->conf.heading = "";
        this->conf.subHeading = "";
        this->conf.allowNegative = true;
        this->conf.allowDecimal = true;

        // Set callback to open keyboard
        this->setCallback([this]() {
            this->openNumpad();
        });

        this->textFunc = nullptr;
    }

    void NumberBox::openNumpad() {
        // Invoke keyboard and update stored text
        this->conf.value = std::stoi(this->text->string());
        if (Utils::NX::getUserInput(this->conf)) {
            this->text->setString(std::to_string(this->conf.value));
            this->positionText();
            if (this->textFunc != nullptr) {
                this->textFunc();
            }
        }
    }

    void NumberBox::positionText() {
        // Position and cut off if too long
        this->text->setY(this->y() + (this->h() - this->text->h())/2);
        int wid = (this->rect->x() + this->rect->w()) - this->text->x() - PADDING;
        if (this->text->texW() > wid) {
            this->text->setMask(0, 0, wid, this->text->texH());
            this->text->setW(wid);
        } else {
            this->text->setMask(0, 0, this->text->texW(), this->text->texH());
        }
    }

    void NumberBox::setBoxColour(Aether::Colour c) {
        this->rect->setColour(c);
    }

    void NumberBox::setTextColour(Aether::Colour c) {
        this->text->setColour(c);
    }

    void NumberBox::setNumpadAllowFloats(bool b) {
        this->conf.allowDecimal = b;
    }

    void NumberBox::setNumpadCallback(std::function<void()> f) {
        this->textFunc = f;
    }

    void NumberBox::setNumpadHint(std::string h) {
        this->conf.heading = h;
    }

    void NumberBox::setNumpadLimit(size_t l) {
        this->conf.maxDigits = l;
    }

    void NumberBox::setNumpadNegative(bool b) {
        this->conf.allowNegative = b;
    }

    void NumberBox::setValue(int v) {
        this->text->setString(std::to_string(v));
        this->positionText();
    }

    int NumberBox::value() {
        return std::stoi(this->text->string());
    }

    SDL_Texture * NumberBox::renderHighlightBG() {
        // No background
        return nullptr;
    }

    SDL_Texture * NumberBox::renderHighlight() {
        return SDLHelper::renderRoundRect(this->w() + 2*(this->hiSize), this->h() + 2*(this->hiSize), HI_CORNER_RAD, this->hiSize);
    }

    SDL_Texture * NumberBox::renderSelection() {
        return SDLHelper::renderFilledRoundRect(this->w(), this->h(), BOX_CORNER_RAD);
    }
};