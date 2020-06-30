#include "NX.hpp"
#include "SearchBox.hpp"

// Box corner radius
#define BOX_CORNER_RAD 18
// Highlight corner radius
#define HI_CORNER_RAD 22
// Height
#define HEIGHT 36

namespace CustomElm {
    SearchBox::SearchBox(int x, int y, int w) : Element(x, y, w, HEIGHT) {
        this->rect = new Aether::Rectangle(this->x(), this->y(), this->w(), this->h(), BOX_CORNER_RAD);
        this->addElement(this->rect);
        this->icon = new Aether::Image(this->x() + this->h()/2 + 5, this->y() + this->h()/2, "romfs:/icons/search.png");
        this->icon->setXY(this->icon->x() - this->icon->w()/2, this->icon->y() - this->icon->h()/2);
        this->addElement(this->icon);
        this->text = new Aether::Text(this->icon->x() + this->icon->w() + 10, 0, "", this->h() - 15);
        this->text->setY(this->y() + (this->h() - this->text->h())/2);
        this->addElement(this->text);

        // Set callback to open keyboard
        this->setCallback([this](){
            std::string s = Utils::NX::getUserInput(32, "Search", "Search", "Search for songs, artists or albums...", "", this->text->string());
            if (s != "") {
                this->text->setString(s);
                this->text->setY(this->y() + (this->h() - this->text->h())/2);
                int wid = (this->rect->x() + this->rect->w()) - this->text->x() - 10;
                if (this->text->texW() > wid) {
                    this->text->setMask(0, 0, wid, this->text->texH());
                    this->text->setW(wid);
                } else {
                    this->text->setMask(0, 0, this->text->texW(), this->text->texH());
                }
            }
        });
    }

    void SearchBox::setBoxColour(Aether::Colour c) {
        this->rect->setColour(c);
    }

    void SearchBox::setIconColour(Aether::Colour c) {
        this->icon->setColour(c);
        this->text->setColour(c);
    }

    std::string SearchBox::string() {
        return this->text->string();
    }

    void SearchBox::renderHighlightBG() {
        // Nothing
    }

    void SearchBox::renderHighlight() {
        SDLHelper::drawRoundRect(this->hiBorder, this->x() - this->hiSize, this->y() - this->hiSize, this->w() + 2*(this->hiSize), this->h() + 2*(this->hiSize), HI_CORNER_RAD, this->hiSize);
    }

    void SearchBox::renderSelection() {
        SDLHelper::drawFilledRoundRect(this->hiSel, this->x(), this->y(), this->w(), this->h(), BOX_CORNER_RAD);
    }
};