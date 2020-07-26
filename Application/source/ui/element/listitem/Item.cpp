#include "ui/element/listitem/Item.hpp"

// Number of pixels either side of the screen to keep textures in memory for
#define THRESHOLD 2000

namespace CustomElm::ListItem {
    SDL_Texture * Item::line = nullptr;

    Item::Item(int h) : Aether::Element(0, 0, 100, h) {
        // Create line texture if it doesn't exist
        if (this->line == nullptr) {
            this->line = SDLHelper::renderFilledRect(500, 1);
        }

        // Ensure other variables are initialized properly
        this->lineColour = Aether::Colour{255, 255, 255, 0};
        this->renderStatus = Status::Waiting;
        this->textures.clear();
    }

    void Item::watchTexture(Aether::Texture * t) {
        t->setHidden(true);
        this->textures.push_back(t);
        this->addElement(t);
    }

    void Item::setLineColour(Aether::Colour c) {
        this->lineColour = c;
    }

    void Item::update(uint32_t dt) {
        // Update children
        Element::update(dt);

        // Take action based on rendering status
        switch (this->renderStatus) {
            // Waiting to render - check position and start if within threshold
            case Status::Waiting:
                if (this->y() + this->h() > -THRESHOLD && this->y() < 720 + THRESHOLD) {
                    for (size_t i = 0; i < this->textures.size(); i++) {
                        this->textures[i]->startRendering();
                    }
                    this->renderStatus = Status::InProgress;
                }
                break;

            case Status::InProgress:
                // Check if all are ready
                for (size_t i = 0; i < this->textures.size(); i++) {
                    if (!this->textures[i]->textureReady()) {
                        return;
                    }
                }

                // If they're all ready show and position
                this->positionItems();
                for (size_t i = 0; i < this->textures.size(); i++) {
                    this->textures[i]->setHidden(false);
                }
                this->renderStatus = Status::Done;

            case Status::Done:
                // Delete textures when outside of threshold to save memory
                if (this->y() + this->h() < -THRESHOLD || this->y() > 720 + THRESHOLD) {
                    for (size_t i = 0; i < this->textures.size(); i++) {
                        this->textures[i]->destroyTexture();
                        this->textures[i]->setHidden(true);
                    }
                    this->renderStatus = Status::Waiting;
                }
                break;
        }
    }

    void Item::render() {
        // Render lines first so highlight will appear on top
        if (this->renderStatus == Status::Done && this->isVisible()) {
            SDLHelper::drawTexture(this->line, this->lineColour, this->x(), this->y(), this->w());
            SDLHelper::drawTexture(this->line, this->lineColour, this->x(), this->y() + this->h(), this->w());
        }

        Element::render();
    }

    void Item::setW(int w) {
        Element::setW(w);
        this->positionItems();
    }
};