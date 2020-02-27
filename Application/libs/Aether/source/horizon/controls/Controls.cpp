#include "Controls.hpp"

namespace Aether {
    Controls::Controls(int x, int y, int w, int h) : Container(x, y, w, h) {

    }

    void Controls::addItem(ControlItem * i) {
        this->items.push_back(i);
        this->addElement(i);
        this->repositionElements();
    }

    bool Controls::removeItem(ControlItem * i) {
        if (Container::removeElement(i)) {
            std::vector<ControlItem *>::iterator it = std::find(this->items.begin(), this->items.end(), i);
            if (it != this->items.end()) {
                this->items.erase(it);
                this->repositionElements();
                return true;
            }
        }
        return false;
    }

    void Controls::removeAllItems() {
        Container::removeAllElements();
        this->items.empty();
    }

    void Controls::repositionElements() {
        // Iterate over children and position with first element being right-most
        int nextX = this->x() + this->w();
        for (size_t i = 0; i < this->items.size(); i++) {
            this->items[i]->setX(nextX - this->items[i]->w());
            nextX = this->items[i]->x();
            this->items[i]->setY(this->y() + (this->h() - this->items[i]->h())/2);
        }
    }

    void Controls::setX(int x) {
        Container::setX(x);
        this->repositionElements();
    }

    void Controls::setY(int y) {
        Container::setY(y);
        this->repositionElements();
    }

    void Controls::setW(int w) {
        Container::setW(w);
        this->repositionElements();
    }

    void Controls::setH(int h) {
        Container::setH(h);
        this->repositionElements();
    }

    Colour Controls::getColour() {
        return this->colour;
    }

    void Controls::setColour(Colour c) {
        this->colour = c;
        for (size_t i = 0; i < this->items.size(); i++) {
            this->items[i]->setColour(c);
        }
    }

    void Controls::setColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        this->setColour(Colour{r, g, b, a});
    }
};