#include "NX.hpp"
#include "SearchBox.hpp"

#define CORNER_RAD 10

namespace CustomElm {
    SearchBox::SearchBox(int x, int y, int w, int h) : Element(x, y, w, h) {
        this->rect = new Aether::Box(this->x(), this->y(), this->w(), this->h(), 3, CORNER_RAD);
        this->addElement(this->rect);
        this->icon = new Aether::Image(this->x() + this->rect->border() + 7, this->y() + this->rect->border() + 7, "romfs:/icons/search.png");
        this->icon->setWH(this->h() - (2 * this->rect->border()) - 14, this->h() - (2 * this->rect->border()) - 14);
        this->addElement(this->icon);
        this->text = new Aether::Text(this->icon->x() + this->icon->w() + 10, this->y() + this->rect->border() + 2, "", this->h() - (2 * this->rect->border()) - 10);
        this->text->setY(this->y() + (this->h() - this->text->h())/2);
        this->addElement(this->text);

        // Set callback to open keyboard
        this->setCallback([this](){
            std::string s = Utils::NX::getUserInput(32, "Search", "Search", "Search for songs, artists or albums...", "", this->text->string());
            if (s != "") {
                this->text->setString(s);
                this->text->setY(this->y() + (this->h() - this->text->h())/2);
            }
        });
    }

    void SearchBox::setColour(Aether::Colour c) {
        this->rect->setColour(c);
        this->icon->setColour(c);
        this->text->setColour(c);
    }

    std::string SearchBox::string() {
        return this->text->string();
    }

    void SearchBox::render() {
        Element::render();

        // Now render highlight over the top
        if (this->highlighted() && !this->isTouch) {
            SDLHelper::drawRoundRect(this->hiBorder, this->x() - this->hiSize + this->rect->border(), this->y() - this->hiSize + this->rect->border(), this->w() + 2*(this->hiSize - this->rect->border()), this->h() + 2*(this->hiSize - this->rect->border()), CORNER_RAD + 2, this->hiSize);
        }
    }

    void SearchBox::renderHighlighted() {
        // Draw background
        SDLHelper::drawFilledRoundRect(this->hiBG, this->x(), this->y(), this->w(), this->h(), CORNER_RAD + 2);
    }

    void SearchBox::renderSelected() {
        SDLHelper::drawFilledRoundRect(this->hiSel, this->x(), this->y(), this->w(), this->h(), CORNER_RAD + 2);
    }
};