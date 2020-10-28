#include "Application.hpp"
#include "ui/frame/Frame.hpp"

// Size and position of all frames
#define X 320
#define Y 0
#define W 960
#define H 590

namespace Frame {
    Frame::Frame(Main::Application * a) : Aether::Container(X, Y, W, H) {
        this->app = a;
        this->changeFrame = nullptr;

        // Create list
        this->bottomContainer = new Aether::Container(this->x(), this->y() + 200, this->w(), this->h() - 200);
        this->list = new Aether::List(this->x(), this->y() + 200, this->w() - 10, this->h() - 200);
        this->list->setScrollBarColour(this->app->theme()->muted2());
        this->list->setShowScrollBar(true);
        this->bottomContainer->addElement(this->list);
        this->bottomContainer->setFocussed(this->list);
        this->addElement(this->bottomContainer);

        // Set up elements
        this->topContainer = new Aether::Container(this->x(), this->y(), this->w(), 200);
        this->heading = new Aether::Text(this->x() + 65, this->y() + 40, "|", 60);
        this->heading->setColour(this->app->theme()->FG());
        this->topContainer->addElement(this->heading);
        this->subHeading = new Aether::Text(this->heading->x() + 2, this->heading->y() + this->heading->h() + 5, "", 20);
        this->subHeading->setColour(this->app->theme()->muted());
        this->topContainer->addElement(this->subHeading);
        this->sort = new Aether::BorderButton(this->x() + 755, this->heading->y() + (this->heading->h() - 56)/2 + 5, 130, 50, 2, "Sort", 26, [this]() {
            // yes we can
        });
        this->sort->setBorderColour(this->app->theme()->FG());
        this->sort->setTextColour(this->app->theme()->FG());
        this->topContainer->addElement(this->sort);
        this->titleH = new Aether::Text(this->x() + 65, this->y() + 170, "Title", 20);
        this->titleH->setColour(this->app->theme()->muted());
        this->topContainer->addElement(this->titleH);
        this->artistH = new Aether::Text(this->x() + 425, this->titleH->y(), "Artist", 20);
        this->artistH->setColour(this->app->theme()->muted());
        this->topContainer->addElement(this->artistH);
        this->albumH = new Aether::Text(this->x() + 620, this->titleH->y(), "Album", 20);
        this->albumH->setColour(this->app->theme()->muted());
        this->topContainer->addElement(this->albumH);
        this->lengthH = new Aether::Text(this->x() + 850, this->titleH->y(), "Length", 20);
        this->lengthH->setX(this->lengthH->x() - this->lengthH->w());
        this->lengthH->setColour(this->app->theme()->muted());
        this->topContainer->addElement(this->lengthH);
        this->addElement(this->topContainer);

        this->setFocussed(this->bottomContainer);
    }

    bool Frame::handleEvent(Aether::InputEvent * e) {
        // Wait for - press
        if (e->type() == Aether::EventType::ButtonPressed && e->button() == Aether::Button::MINUS) {
            if (this->focussed() == this->topContainer && this->bottomContainer->hasSelectable()) {
                this->setFocussed(this->bottomContainer);
                return true;

            } else if (this->focussed() == this->bottomContainer && this->topContainer->hasSelectable()) {
                this->setFocussed(this->topContainer);
                return true;
            }
        }

        return Container::handleEvent(e);
    }

    void Frame::updateColours() {
        // Do nothing by default
    }

    void Frame::onPush(Type t) {
        // Do nothing by default
    }

    void Frame::onPop(Type t) {
        // Do nothing by default
    }

    void Frame::setChangeFrameFunc(std::function<void(Type, Action, int)> f) {
        this->changeFrame = f;
    }

    void Frame::setPlayNewQueueFunc(std::function<void(const std::string &, const std::vector<SongID> &, const size_t, const bool)> f) {
        this->playNewQueue = f;
    }

    void Frame::setShowAddToPlaylistFunc(std::function<void(std::function<void(PlaylistID)>)> f) {
        this->showAddToPlaylist = f;
    }
};