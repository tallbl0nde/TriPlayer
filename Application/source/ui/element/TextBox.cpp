#include "ui/element/TextBox.hpp"

// Box corner radius
#define BOX_CORNER_RAD 10
// Highlight corner radius
#define HI_CORNER_RAD 16
// Padding around text
#define PADDING 13

namespace CustomElm {
    TextBox::TextBox(int x, int y, int w, int h) : Element(x, y, w, h) {
        this->rect = new Aether::Rectangle(this->x(), this->y(), this->w(), this->h(), BOX_CORNER_RAD);
        this->addElement(this->rect);
        this->text = new Aether::Text(this->x() + PADDING, 0, "", this->h() - 2*PADDING);
        this->text->setY(this->y() + (this->h() - this->text->h())/2);
        this->addElement(this->text);

        // Set initial keyboard config
        this->conf.buffer = "";
        this->conf.hint = "";
        this->conf.maxLength = 100;
        this->conf.ok = "OK";
        this->conf.showLine = false;

        // Set callback to open keyboard
        this->onPress([this]() {
            this->openKeyboard();
        });

        this->textFunc = nullptr;
    }

    void TextBox::openKeyboard() {
        // Invoke keyboard and update stored text
        this->conf.buffer = this->text->string();
        if (Utils::NX::getUserInput(this->conf)) {
            this->text->setString(this->conf.buffer);
            this->positionText();
            if (this->textFunc != nullptr) {
                this->textFunc();
            }
        }
    }

    void TextBox::positionText() {
        // Position and cut off if too long
        this->text->setY(this->y() + (this->h() - this->text->h())/2);
        int wid = (this->rect->x() + this->rect->w()) - this->text->x() - PADDING;
        if (this->text->textureWidth() > wid) {
            this->text->setMask(0, 0, wid, this->text->textureHeight());
            this->text->setW(wid);
        } else {
            this->text->setMask(0, 0, this->text->textureWidth(), this->text->textureHeight());
        }
    }

    void TextBox::setBoxColour(Aether::Colour c) {
        this->rect->setColour(c);
    }

    void TextBox::setTextColour(Aether::Colour c) {
        this->text->setColour(c);
    }

    void TextBox::setKeyboardCallback(std::function<void()> f) {
        this->textFunc = f;
    }

    void TextBox::setKeyboardHint(std::string s) {
        this->conf.hint = s;
    }

    void TextBox::setKeyboardLimit(size_t l) {
        this->conf.maxLength = l;
    }

    void TextBox::setString(std::string s) {
        this->text->setString(s);
        this->positionText();
    }

    std::string TextBox::string() {
        return this->text->string();
    }

    Aether::Drawable * TextBox::renderHighlightBG() {
        // No background
        return new Aether::Drawable();
    }

    Aether::Drawable * TextBox::renderHighlight() {
        return this->renderer->renderRoundRectTexture(this->w() + 2*(this->hiSize), this->h() + 2*(this->hiSize), HI_CORNER_RAD, this->hiSize);
    }

    Aether::Drawable * TextBox::renderSelection() {
        return this->renderer->renderFilledRoundRectTexture(this->w(), this->h(), BOX_CORNER_RAD);
    }
};
