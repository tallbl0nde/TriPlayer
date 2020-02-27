#include "PopupList.hpp"
#include "horizon/Tick.hpp"

// Height of items in list
#define ITEM_HEIGHT 62
// Width of list element
#define LIST_WIDTH 725
// Maximum height of popup
#define MAX_HEIGHT 480
// Title text size
#define TITLE_FONT_SIZE 26

namespace Aether {
    PopupList::PopupList(std::string s) : Overlay() {
        // Constructor initialises elements (starts with smallest height)
        this->rect = new Rectangle(this->x(), this->y() + this->h() - 224, this->w(), 224);
        this->addElement(this->rect);
        this->top = new Rectangle(this->x() + 30, this->rect->y() + 72, this->w() - 60, 1);
        this->addElement(this->top);
        this->bottom = new Rectangle(this->x() + 30, this->rect->y() + this->rect->h() - 72, this->w() - 60, 1);
        this->addElement(this->bottom);

        this->title = new Text(this->x() + 72, this->rect->y() + 40, s, TITLE_FONT_SIZE);
        this->title->setY(this->title->y() - this->title->h()/2);
        this->addElement(this->title);

        this->ctrl = new Controls();
        ControlItem * i = new ControlItem(Button::A, "OK");
        this->ctrl->addItem(i);
        i = new ControlItem(Button::B, "Back");
        this->ctrl->addItem(i);
        this->addElement(this->ctrl);

        this->list = new List((this->w() - LIST_WIDTH)/2, this->top->y(), LIST_WIDTH, 80);
        this->addElement(this->list);
        this->setFocussed(this->list);

        this->onButtonPress(Button::B, [this](){
            this->close(true);
        });
    }

    void PopupList::addEntry(std::string s, std::function<void()> f, bool t) {
        // Create and add element to list
        ListButton * b = new ListButton(s, [f, this](){
            // Call callback
            f();
            // Close
            this->close(true);
        });
        b->setH(ITEM_HEIGHT);
        b->setLineColour(this->llColour);
        b->setTextColour(this->txColour);
        this->items.push_back(b);
        this->list->addElement(b);
        if (t) {
            b->setTextColour(this->hiColour);
            Tick * tick = new Tick(b->x() + b->w() - 16, b->y() + b->h()/2, 26);
            tick->setXY(tick->x() - tick->w(), tick->y() - tick->h()/2);
            tick->setCircleColour(this->hiColour);
            tick->setTickColour(this->rect->getColour());
            b->addElement(tick);
        }

        // Increase height if need be
        if (this->rect->h() < MAX_HEIGHT) {
            this->rect->setY(this->rect->y() - ITEM_HEIGHT);
            this->rect->setH(this->rect->h() + ITEM_HEIGHT);
            this->title->setY(this->title->y() - ITEM_HEIGHT);
            this->top->setY(this->top->y() - ITEM_HEIGHT);
            this->list->setY(this->list->y() - ITEM_HEIGHT);
            this->list->setH(this->list->h() + ITEM_HEIGHT);
        }
    }

    void PopupList::removeEntries() {
        this->items.empty();
        this->list->removeAllElements();

        // Reset heights
        this->rect->setY(this->y() + this->h() - 224);
        this->rect->setH(224);
        this->title->setY(this->rect->y() + 40 - this->title->h()/2);
        this->top->setY(this->rect->y() + 72);
        this->list->setY(this->top->y());
        this->list->setH(80);
    }

    Colour PopupList::getBackgroundColour() {
        return this->rect->getColour();
    }

    void PopupList::setBackgroundColour(Colour c) {
        this->rect->setColour(c);
    }

    Colour PopupList::getTextColour() {
        return this->txColour;
    }

    void PopupList::setTextColour(Colour c) {
        this->txColour = c;
        this->ctrl->setColour(c);
        this->title->setColour(c);
        for (size_t i = 0; i < this->items.size(); i++) {
            this->items[i]->setTextColour(c);
        }
    }

    Colour PopupList::getLineColour() {
        return this->top->getColour();
    }

    void PopupList::setLineColour(Colour c) {
        this->top->setColour(c);
        this->bottom->setColour(c);
    }

    Colour PopupList::getHighlightColour() {
        return this->hiColour;
    }

    void PopupList::setHighlightColour(Colour c) {
        this->hiColour = c;
    }

    Colour PopupList::getListLineColour() {
        return this->llColour;
    }

    void PopupList::setListLineColour(Colour c) {
        this->llColour = c;
        for (size_t i = 0; i < this->items.size(); i++) {
            this->items[i]->setLineColour(c);
        }
        this->list->setScrollBarColour(c);
    }

    void PopupList::setAllColours(Colour bg, Colour hi, Colour li, Colour lli, Colour tx) {
        this->setBackgroundColour(bg);
        this->setHighlightColour(hi);
        this->setLineColour(li);
        this->setListLineColour(lli);
        this->setTextColour(tx);
    }
};