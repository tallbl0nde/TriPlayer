#include "ui/element/listitem/Item.hpp"

// Number of pixels either side of the screen to keep textures in memory for
#define THRESHOLD 2000

namespace CustomElm::ListItem {
    Aether::Drawable * Item::line = new Aether::Drawable();

    Item::Item(int h) : Aether::AsyncItem() {
        this->setH(h);

        // Create line texture if it doesn't exist
        if (this->line->type() == Aether::Drawable::Type::None) {
            this->line = this->renderer->renderFilledRectTexture(500, 1);
        }
    }

    void Item::processText(Aether::Text * & text, std::function<Aether::Text * ()> getNew) {
        // Remove original
        this->removeTexture(text);
        this->removeElement(text);

        // Get (and assign) new text object
        text = getNew();

        // Don't add if empty string
        if (!text->string().empty()) {
            this->addElement(text);
            this->addTexture(text);
        }
    }

    void Item::setLineColour(const Aether::Colour & c) {
        this->line->setColour(c);
    }

    void Item::render() {
        // Render lines first so highlight will appear on top
        if (this->ready() && this->isVisible()) {
            this->line->render(this->x(), this->y(), this->w());
            this->line->render(this->x(), this->y() + this->h(), this->w());
        }

        AsyncItem::render();
    }
};
