#include "ui/overlay/menu/Song.hpp"

// Size vars
#define ITEM_HEIGHT 60
#define ITEM_INDENT 20

namespace CustomOvl::Menu {
    Song::Song(Song::Type t) : Menu(::CustomOvl::Menu::Type::Normal) {
        // Top section (song)
        this->album = nullptr;
        this->title = new Aether::Text(this->bg->x() + 130, this->bg->y() + 33, "", 24);
        this->title->setScroll(true);
        this->title->setScrollSpeed(35);
        this->title->setScrollWaitTime(1200);
        this->bg->addElement(this->title);
        this->artist = new Aether::Text(this->title->x(), this->title->y() + 34, "", 18);
        this->bg->addElement(this->artist);

        // Buttons are placed in a container for easy moving
       this->btns = new Aether::Container(this->bg->x(), this->bg->y() + 120 + 5, this->bg->w(), 720);

        // Queue/Playlist section
        int h = this->btns->y();
        if (t == Song::Type::ShowRemove) {
            this->removeFromQueue = new CustomElm::MenuButton(this->bg->x() + ITEM_INDENT, h, this->bg->w() - 2*ITEM_INDENT, ITEM_HEIGHT);
            this->removeFromQueue->setIcon(new Aether::Image(0, 0, "romfs:/icons/removefromqueue.png"));
            this->removeFromQueue->setCallback(nullptr);
            this->btns->addElement(this->removeFromQueue);
            h += ITEM_HEIGHT;
        } else {
            this->removeFromQueue = nullptr;
        }

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

        // Go to section
        h += 10;
        this->goToArtist = new CustomElm::MenuButton(this->bg->x() + ITEM_INDENT, h, this->bg->w() - 2*ITEM_INDENT, ITEM_HEIGHT);
        this->goToArtist->setIcon(new Aether::Image(0, 0, "romfs:/icons/user.png"));
        this->goToArtist->setCallback(nullptr);
        this->btns->addElement(this->goToArtist);
        h += ITEM_HEIGHT;

        this->goToAlbum = new CustomElm::MenuButton(this->bg->x() + ITEM_INDENT, h, this->bg->w() - 2*ITEM_INDENT, ITEM_HEIGHT);
        this->goToAlbum->setIcon(new Aether::Image(0, 0, "romfs:/icons/disc.png"));
        this->goToAlbum->setCallback(nullptr);
        this->btns->addElement(this->goToAlbum);
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
        this->btns->setXY(this->bg->x(), this->bg->y() + 120 + 5);
        this->btns->setH(h - this->btns->y());
        this->addElement(this->btns);
    }

    void Song::resetHighlight() {
        if (this->removeFromQueue == nullptr) {
            this->btns->setFocussed(this->addToQueue);
        } else {
            this->btns->setFocussed(this->removeFromQueue);
        }
    }

    void Song::setAlbum(Aether::Image * i) {
        if (this->album != nullptr) {
            this->removeElement(this->album);
        }
        this->album = i;
        this->album->setXY(this->bg->x() + 25, this->bg->y() + 20);
        this->album->setWH(85, 85);
        this->addElement(this->album);
    }

    void Song::setTitle(std::string s) {
        this->title->setString(s);
        if (this->title->w() > (this->bg->x() + this->bg->w()) - this->title->x() - ITEM_INDENT) {
            this->title->setW((this->bg->x() + this->bg->w()) - this->title->x() - ITEM_INDENT);
        }
    }

    void Song::setArtist(std::string s) {
        this->artist->setString(s);
        if (this->artist->w() > (this->bg->x() + this->bg->w()) - this->artist->x() - ITEM_INDENT) {
            this->artist->setW((this->bg->x() + this->bg->w()) - this->artist->x() - ITEM_INDENT);
        }
    }

    void Song::setRemoveFromQueueText(std::string s) {
        if (this->removeFromQueue != nullptr) {
            this->removeFromQueue->setText(s);
        }
    }

    void Song::setAddToQueueText(std::string s) {
        this->addToQueue->setText(s);
    }

    void Song::setAddToPlaylistText(std::string s) {
        this->addToPlaylist->setText(s);
    }

    void Song::setGoToArtistText(std::string s) {
        this->goToArtist->setText(s);
    }

    void Song::setGoToAlbumText(std::string s) {
        this->goToAlbum->setText(s);
    }

    void Song::setViewInformationText(std::string s) {
        this->viewInformation->setText(s);
    }

    void Song::setRemoveFromQueueFunc(std::function<void()> f) {
        if (this->removeFromQueue != nullptr) {
            this->removeFromQueue->setCallback(f);
        }
    }

    void Song::setAddToQueueFunc(std::function<void()> f) {
        this->addToQueue->setCallback(f);
    }

    void Song::setAddToPlaylistFunc(std::function<void()> f) {
        this->addToPlaylist->setCallback(f);
    }

    void Song::setGoToArtistFunc(std::function<void()> f) {
        this->goToArtist->setCallback(f);
    }

    void Song::setGoToAlbumFunc(std::function<void()> f) {
        this->goToAlbum->setCallback(f);
    }

    void Song::setViewInformationFunc(std::function<void()> f) {
        this->viewInformation->setCallback(f);
    }

    void Song::setIconColour(Aether::Colour c) {
        if (this->removeFromQueue != nullptr) {
            this->removeFromQueue->setIconColour(c);
        }
        this->addToQueue->setIconColour(c);
        this->addToPlaylist->setIconColour(c);
        this->goToArtist->setIconColour(c);
        this->goToAlbum->setIconColour(c);
        this->viewInformation->setIconColour(c);
    }

    void Song::setMutedTextColour(Aether::Colour c) {
        this->artist->setColour(c);
    }

    void Song::setTextColour(Aether::Colour c) {
        this->title->setColour(c);
        if (this->removeFromQueue != nullptr) {
            this->removeFromQueue->setTextColour(c);
        }
        this->addToQueue->setTextColour(c);
        this->addToPlaylist->setTextColour(c);
        this->goToArtist->setTextColour(c);
        this->goToAlbum->setTextColour(c);
        this->viewInformation->setTextColour(c);
    }

    void Song::render() {
        Menu::render();

        if (this->removeFromQueue != nullptr) {
            if (this->isTouch || !this->removeFromQueue->highlighted()) {
                SDLHelper::drawTexture(this->line, this->lineColour, this->bg->x() + 20, this->removeFromQueue->y() - 5);
            }
        } else if (this->isTouch || !this->addToQueue->highlighted()) {
            SDLHelper::drawTexture(this->line, this->lineColour, this->bg->x() + 20, this->addToQueue->y() - 5);
        }

        if (this->isTouch || (!this->addToPlaylist->highlighted() && !this->goToArtist->highlighted())) {
            SDLHelper::drawTexture(this->line, this->lineColour, this->bg->x() + 20, this->goToArtist->y() - 5);
        }
        if (this->isTouch || (!this->goToAlbum->highlighted() && !this->viewInformation->highlighted())) {
            SDLHelper::drawTexture(this->line, this->lineColour, this->bg->x() + 20, this->viewInformation->y() - 5);
        }
    }
};