#include "ui/overlay/menu/Album.hpp"

// Dimension variables
#define ITEM_HEIGHT 60
#define ITEM_INDENT 20

namespace CustomOvl::Menu {
    Album::Album(Type t) : Menu(t) {
        // Top section
        this->image = nullptr;
        int offsetY = 5;
        if (this->type == Type::Normal) {
            this->name = new Aether::Text(this->bg->x() + 130, this->bg->y() + 33, "", 24);
            this->name->setScroll(true);
            this->name->setScrollSpeed(35);
            this->name->setScrollWaitTime(1200);
            this->bg->addElement(this->name);
            this->stats = new Aether::Text(this->name->x(), this->name->y() + 34, "", 18);
            this->bg->addElement(this->stats);
            offsetY = 120;

        } else {
            this->name = nullptr;
            this->stats = nullptr;
        }

        // Buttons are placed in a container for easy moving
        this->btns = new Aether::Container(this->bg->x(), this->bg->y() + offsetY + 5, this->bg->w(), 720);

        // Play All
        int h = this->btns->y();

        this->playAll = new CustomElm::MenuButton(this->bg->x() + ITEM_INDENT, h, this->bg->w() - 2*ITEM_INDENT, ITEM_HEIGHT);
        this->playAll->setIcon(new Aether::Image(0, 0, "romfs:/icons/playsmall.png"));
        this->playAll->setCallback(nullptr);
        this->btns->addElement(this->playAll);
        h += ITEM_HEIGHT;

        // Add to...
        h += 10;
        this->addToQueue = new CustomElm::MenuButton(this->bg->x() + ITEM_INDENT, h, this->bg->w() - 2*ITEM_INDENT, ITEM_HEIGHT);
        this->addToQueue->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoqueue.png"));
        this->addToQueue->setCallback(nullptr);
        this->btns->addElement(this->addToQueue);
        h += ITEM_HEIGHT;

        this->addToPlaylist = new CustomElm::MenuButton(this->bg->x() + ITEM_INDENT, h, this->bg->w() - 2*ITEM_INDENT, ITEM_HEIGHT);
        this->addToPlaylist->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoplaylist.png"));
        this->addToPlaylist->setCallback(nullptr);
        this->btns->addElement(this->addToPlaylist);
        h += ITEM_HEIGHT;

        // Go to artist
        h += 10;
        this->goToArtist = new CustomElm::MenuButton(this->bg->x() + ITEM_INDENT, h, this->bg->w() - 2*ITEM_INDENT, ITEM_HEIGHT);
        this->goToArtist->setIcon(new Aether::Image(0, 0, "romfs:/icons/user.png"));
        this->goToArtist->setCallback(nullptr);
        this->btns->addElement(this->goToArtist);
        h += ITEM_HEIGHT;

        // Details
        h += 10;
        this->viewInformation = new CustomElm::MenuButton(this->bg->x() + ITEM_INDENT, h, this->bg->w() - 2*ITEM_INDENT, ITEM_HEIGHT);
        this->viewInformation->setIcon(new Aether::Image(0, 0, "romfs:/icons/info.png"));
        this->viewInformation->setCallback(nullptr);
        this->btns->addElement(this->viewInformation);
        h += ITEM_HEIGHT;

        // Set background properly now
        this->bg->setRectSize(this->bg->w(), h + 10 - this->bg->y());
        this->bg->setXY(640 - this->bg->w()/2, 360 - this->bg->h()/2);

        // Position buttons
        this->btns->setXY(this->bg->x(), this->bg->y() + offsetY + 5);
        this->btns->setH(h - this->btns->y());
        this->addElement(this->btns);
    }

    void Album::resetHighlight() {
        this->btns->setFocussed(this->playAll);
    }

    void Album::setImage(Aether::Image * i) {
        if (this->type == Type::HideTop) {
            return;
        }

        if (this->image != nullptr) {
            this->removeElement(this->image);
        }
        this->image = i;
        this->image->setXY(this->bg->x() + 25, this->bg->y() + 20);
        this->image->setWH(85, 85);
        this->addElement(this->image);
    }

    void Album::setName(std::string s) {
        if (this->type == Type::HideTop) {
            return;
        }

        this->name->setString(s);
        if (this->name->w() > (this->bg->x() + this->bg->w()) - this->name->x() - ITEM_INDENT) {
            this->name->setW((this->bg->x() + this->bg->w()) - this->name->x() - ITEM_INDENT);
        }
    }

    void Album::setStats(std::string s) {
        if (this->type == Type::HideTop) {
            return;
        }

        this->stats->setString(s);
        if (this->stats->w() > (this->bg->x() + this->bg->w()) - this->stats->x() - ITEM_INDENT) {
            this->stats->setW((this->bg->x() + this->bg->w()) - this->stats->x() - ITEM_INDENT);
        }
    }

    void Album::setPlayAllText(std::string s) {
        this->playAll->setText(s);
    }

    void Album::setAddToQueueText(std::string s) {
        this->addToQueue->setText(s);
    }

    void Album::setAddToPlaylistText(std::string s) {
        this->addToPlaylist->setText(s);
    }

    void Album::setGoToArtistText(std::string s) {
        this->goToArtist->setText(s);
    }

    void Album::setViewInformationText(std::string s) {
        this->viewInformation->setText(s);
    }

    void Album::setPlayAllFunc(std::function<void()> f) {
        this->playAll->setCallback(f);
    }

    void Album::setAddToQueueFunc(std::function<void()> f) {
        this->addToQueue->setCallback(f);
    }

    void Album::setAddToPlaylistFunc(std::function<void()> f) {
        this->addToPlaylist->setCallback(f);
    }

    void Album::setGoToArtistFunc(std::function<void()> f) {
        this->goToArtist->setCallback(f);
    }

    void Album::setViewInformationFunc(std::function<void()> f) {
        this->viewInformation->setCallback(f);
    }

    void Album::setIconColour(Aether::Colour c) {
        this->playAll->setIconColour(c);
        this->addToQueue->setIconColour(c);
        this->addToPlaylist->setIconColour(c);
        this->goToArtist->setIconColour(c);
        this->viewInformation->setIconColour(c);
    }

    void Album::setMutedTextColour(Aether::Colour c) {
        if (this->type == Type::HideTop) {
            return;
        }

        this->stats->setColour(c);
    }

    void Album::setTextColour(Aether::Colour c) {
        this->playAll->setTextColour(c);
        this->addToQueue->setTextColour(c);
        this->addToPlaylist->setTextColour(c);
        this->goToArtist->setTextColour(c);
        this->viewInformation->setTextColour(c);
    }

    void Album::render() {
        Menu::render();

        if (this->type == Type::Normal) {
            if (this->isTouch || !this->playAll->highlighted()) {
                SDLHelper::drawTexture(this->line, this->lineColour, this->bg->x() + 20, this->playAll->y() - 5);
            }
        }
        if (this->isTouch || (!this->playAll->highlighted() && !this->addToQueue->highlighted())) {
            SDLHelper::drawTexture(this->line, this->lineColour, this->bg->x() + 20, this->addToQueue->y() - 5);
        }
        if (this->isTouch || (!this->addToPlaylist->highlighted() && !this->goToArtist->highlighted())) {
            SDLHelper::drawTexture(this->line, this->lineColour, this->bg->x() + 20, this->goToArtist->y() - 5);
        }
        if (this->isTouch || (!this->goToArtist->highlighted() && !this->viewInformation->highlighted())) {
            SDLHelper::drawTexture(this->line, this->lineColour, this->bg->x() + 20, this->viewInformation->y() - 5);
        }
    }
};