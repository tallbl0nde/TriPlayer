#include "HelpButton.hpp"

namespace Aether {
    HelpButton::HelpButton(int x, int y, int d, std::function<void()> f) : Element(x, y, d, d) {
        this->text = new Text(x, y, "\uE142", d, FontType::Extended);
        this->addElement(this->text);
        this->setCallback(f);
    }

    void HelpButton::renderHighlighted() {
        // Draw outline
        SDLHelper::drawEllipse(this->hiBorder, this->x() + this->w()/2, this->y() + this->h()/2, this->w() + 2*this->hiSize, this->h() + 2*this->hiSize);

        // Draw background
        SDLHelper::drawEllipse(this->hiBG, this->x() + this->w()/2, this->y() + this->h()/2, this->w(), this->h());
    }

    void HelpButton::renderSelected() {
        SDLHelper::drawEllipse(this->hiSel, this->x() + this->w()/2, this->y() + this->h()/2, this->w(), this->h());
    }

    Colour HelpButton::getColour() {
        return this->text->getColour();
    }

    void HelpButton::setColour(Colour c) {
        this->text->setColour(c);
    }
};