#include "SongMenu.hpp"

// Size vars
#define W 380
#define ITEM_HEIGHT 60
#define ITEM_INDENT 20

namespace CustomOvl {
    SongMenu::SongMenu(bool showRemove) : Aether::Overlay() {
        // Close if B pressed
        this->onButtonPress(Aether::Button::B, [this]() {
            this->close();
        });

        // Add a second transparent layer cause we like it dark ;0
        Aether::Rectangle * r = new Aether::Rectangle(this->x(), this->y(), this->w(), this->h());
        r->setColour(Aether::Colour{0, 0, 0, 120});
        this->addElement(r);

        // Background (will resize afterwards)
        this->bg = new Aether::Rectangle(0, 0, W, 1, 25);
        this->addElement(this->bg);

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
        Aether::Container * btns = new Aether::Container(this->bg->x(), this->bg->y() + 120 + 5, this->bg->w(), 720);

        // Queue/Playlist section
        int h = btns->y();
        if (showRemove) {
            this->removeFromQueue = new CustomElm::MenuButton(this->bg->x() + ITEM_INDENT, h, this->bg->w() - 2*ITEM_INDENT, ITEM_HEIGHT);
            this->removeFromQueue->setIcon(new Aether::Image(0, 0, "romfs:/icons/removefromqueue.png"));
            this->removeFromQueue->setCallback(nullptr);
            btns->addElement(this->removeFromQueue);
            h += ITEM_HEIGHT;
        } else {
            this->removeFromQueue = nullptr;
        }

        this->addToQueue = new CustomElm::MenuButton(this->bg->x() + ITEM_INDENT, h, this->bg->w() - 2*ITEM_INDENT, ITEM_HEIGHT);
        this->addToQueue->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoqueue.png"));
        this->addToQueue->setCallback(nullptr);
        btns->addElement(this->addToQueue);
        h += ITEM_HEIGHT;

        this->addToPlaylist = new CustomElm::MenuButton(this->bg->x() + ITEM_INDENT, h, this->bg->w() - 2*ITEM_INDENT, ITEM_HEIGHT);
        this->addToPlaylist->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoplaylist.png"));
        this->addToPlaylist->setCallback(nullptr);
        btns->addElement(this->addToPlaylist);
        h += ITEM_HEIGHT;

        // Go to section
        h += 10;
        this->goToArtist = new CustomElm::MenuButton(this->bg->x() + ITEM_INDENT, h, this->bg->w() - 2*ITEM_INDENT, ITEM_HEIGHT);
        this->goToArtist->setIcon(new Aether::Image(0, 0, "romfs:/icons/user.png"));
        this->goToArtist->setCallback(nullptr);
        btns->addElement(this->goToArtist);
        h += ITEM_HEIGHT;

        this->goToAlbum = new CustomElm::MenuButton(this->bg->x() + ITEM_INDENT, h, this->bg->w() - 2*ITEM_INDENT, ITEM_HEIGHT);
        this->goToAlbum->setIcon(new Aether::Image(0, 0, "romfs:/icons/disc.png"));
        this->goToAlbum->setCallback(nullptr);
        btns->addElement(this->goToAlbum);
        h += ITEM_HEIGHT;

        // Details
        h += 10;
        this->viewDetails = new CustomElm::MenuButton(this->bg->x() + ITEM_INDENT, h, this->bg->w() - 2*ITEM_INDENT, ITEM_HEIGHT);
        this->viewDetails->setIcon(new Aether::Image(0, 0, "romfs:/icons/info.png"));
        this->viewDetails->setCallback(nullptr);
        btns->addElement(this->viewDetails);
        h += ITEM_HEIGHT;

        // Set background properly now
        this->bg->setRectSize(this->bg->w(), h + 10 - this->bg->y());
        this->bg->setXY(640 - this->bg->w()/2, 360 - this->bg->h()/2);

        // Position buttons
        btns->setXY(this->bg->x(), this->bg->y() + 120 + 5);
        btns->setH(h - btns->y());
        this->addElement(btns);

        // Create line texture
        this->line = SDLHelper::renderFilledRect(this->bg->w() - 40, 1);
        this->lineCol = Aether::Colour{255, 255, 255, 255};
    }

    void SongMenu::setAlbum(Aether::Image * i) {
        if (this->album != nullptr) {
            this->removeElement(this->album);
        }
        this->album = i;
        this->album->setXY(this->bg->x() + 25, this->bg->y() + 20);
        this->album->setWH(85, 85);
        this->addElement(this->album);
    }

    void SongMenu::setTitle(std::string s) {
        this->title->setString(s);
        if (this->title->w() > (this->bg->x() + this->bg->w()) - this->title->x() - ITEM_INDENT) {
            this->title->setW((this->bg->x() + this->bg->w()) - this->title->x() - ITEM_INDENT);
        }
    }

    void SongMenu::setArtist(std::string s) {
        this->artist->setString(s);
        if (this->artist->w() > (this->bg->x() + this->bg->w()) - this->artist->x() - ITEM_INDENT) {
            this->artist->setW((this->bg->x() + this->bg->w()) - this->artist->x() - ITEM_INDENT);
        }
    }

    void SongMenu::setRemoveFromQueueText(std::string s) {
        if (this->removeFromQueue != nullptr) {
            this->removeFromQueue->setText(s);
        }
    }

    void SongMenu::setAddToQueueText(std::string s) {
        this->addToQueue->setText(s);
    }

    void SongMenu::setAddToPlaylistText(std::string s) {
        this->addToPlaylist->setText(s);
    }

    void SongMenu::setGoToArtistText(std::string s) {
        this->goToArtist->setText(s);
    }

    void SongMenu::setGoToAlbumText(std::string s) {
        this->goToAlbum->setText(s);
    }

    void SongMenu::setViewDetailsText(std::string s) {
        this->viewDetails->setText(s);
    }

    void SongMenu::setRemoveFromQueueFunc(std::function<void()> f) {
        this->removeFromQueue->setCallback(f);
    }

    void SongMenu::setAddToQueueFunc(std::function<void()> f) {
        this->addToQueue->setCallback(f);
    }

    void SongMenu::setAddToPlaylistFunc(std::function<void()> f) {
        this->addToPlaylist->setCallback(f);
    }

    void SongMenu::setGoToArtistFunc(std::function<void()> f) {
        this->goToArtist->setCallback(f);
    }

    void SongMenu::setGoToAlbumFunc(std::function<void()> f) {
        this->goToAlbum->setCallback(f);
    }

    void SongMenu::setViewDetailsFunc(std::function<void()> f) {
        this->viewDetails->setCallback(f);
    }

    void SongMenu::setBackgroundColour(Aether::Colour c) {
        this->bg->setColour(c);
    }

    void SongMenu::setIconColour(Aether::Colour c) {
        if (this->removeFromQueue != nullptr) {
            this->removeFromQueue->setIconColour(c);
        }
        this->addToQueue->setIconColour(c);
        this->addToPlaylist->setIconColour(c);
        this->goToArtist->setIconColour(c);
        this->goToAlbum->setIconColour(c);
        this->viewDetails->setIconColour(c);
    }

    void SongMenu::setLineColour(Aether::Colour c) {
        this->lineCol = c;
    }

    void SongMenu::setMutedTextColour(Aether::Colour c) {
        this->artist->setColour(c);
    }

    void SongMenu::setTextColour(Aether::Colour c) {
        this->title->setColour(c);
        if (this->removeFromQueue != nullptr) {
            this->removeFromQueue->setTextColour(c);
        }
        this->addToQueue->setTextColour(c);
        this->addToPlaylist->setTextColour(c);
        this->goToArtist->setTextColour(c);
        this->goToAlbum->setTextColour(c);
        this->viewDetails->setTextColour(c);
    }

    bool SongMenu::handleEvent(Aether::InputEvent * e) {
        if (Overlay::handleEvent(e)) {
            return true;
        }

        if (e->type() == Aether::EventType::TouchReleased) {
            if (e->touchX() < this->bg->x() || e->touchX() > this->bg->x() + this->bg->w() || e->touchY() < this->bg->y() || e->touchY() > this->bg->y() + this->bg->h()) {
                this->close();
                return true;
            }
        }

        return false;
    }

    void SongMenu::render() {
        Overlay::render();

        if (this->removeFromQueue != nullptr) {
            if (this->isTouch || !this->removeFromQueue->highlighted()) {
                SDLHelper::drawTexture(this->line, this->lineCol, this->bg->x() + 20, this->removeFromQueue->y() - 5);
            }
        } else if (this->isTouch || !this->addToQueue->highlighted()) {
            SDLHelper::drawTexture(this->line, this->lineCol, this->bg->x() + 20, this->addToQueue->y() - 5);
        }

        if (this->isTouch || (!this->addToPlaylist->highlighted() && !this->goToArtist->highlighted())) {
            SDLHelper::drawTexture(this->line, this->lineCol, this->bg->x() + 20, this->goToArtist->y() - 5);
        }
        if (this->isTouch || (!this->goToAlbum->highlighted() && !this->viewDetails->highlighted())) {
            SDLHelper::drawTexture(this->line, this->lineCol, this->bg->x() + 20, this->viewDetails->y() - 5);
        }
    }

    SongMenu::~SongMenu() {
        SDLHelper::destroyTexture(this->line);
    }
};