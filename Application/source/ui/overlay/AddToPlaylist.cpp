#include "ui/overlay/AddToPlaylist.hpp"

// Heading text size
#define HEADING_SIZE 30
// Fixed size of overlay
#define HEIGHT 600
#define WIDTH 500
// Padding around heading
#define PADDING 30

namespace CustomOvl {
    AddToPlaylist::AddToPlaylist() : Overlay() {
        // Background rectangle has fixed size
        this->bg = new Aether::Rectangle(this->x() + (this->w() - WIDTH)/2, this->y() + (this->h() - HEIGHT)/2, WIDTH, HEIGHT, 25);
        this->addElement(this->bg);
        this->setTopLeft(this->bg->x(), this->bg->y());
        this->setBottomRight(this->bg->x() + this->bg->w(), this->bg->y() + this->bg->h());

        // Create heading text
        this->heading = new Aether::Text(this->bg->x() + PADDING, this->bg->y() + PADDING, "|", HEADING_SIZE);
        this->addElement(this->heading);

        // Line separating heading and list
        this->line = new Aether::Rectangle(this->bg->x(), this->bg->y() + 2*PADDING + HEADING_SIZE + 5, this->bg->w(), 1);
        this->addElement(this->line);

        // Create and add empty list (no scrollbar)
        this->list = new Aether::List(this->bg->x() - 25, this->line->y(), this->bg->w() + 50, this->bg->y() + this->bg->h() - this->line->y());
        this->list->setShowScrollBar(false);
        this->addElement(this->list);
        this->setFocussed(this->list);

        this->chosenCallback = nullptr;
    }

    void AddToPlaylist::setBackgroundColour(Aether::Colour c) {
        this->bg->setColour(c);
    }

    void AddToPlaylist::setHeadingColour(Aether::Colour c) {
        this->heading->setColour(c);
    }

    void AddToPlaylist::setHeadingString(const std::string & s) {
        this->heading->setString(s);
    }

    void AddToPlaylist::setLineColour(Aether::Colour c) {
        this->line->setColour(c);
    }

    void AddToPlaylist::addPlaylist(CustomElm::ListItem::Playlist * l, PlaylistID id) {
        l->setCallback([this, id]() {
            if (this->chosenCallback != nullptr) {
                this->chosenCallback(id);
            }
            this->close();
        });
        this->list->addElement(l);
    }

    void AddToPlaylist::setChosenCallback(std::function<void(PlaylistID)> f) {
        this->chosenCallback = f;
    }
};