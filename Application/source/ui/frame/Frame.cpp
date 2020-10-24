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

        // Set up elements
        this->heading = new Aether::Text(this->x() + 65, this->y() + 40, "", 60);
        this->heading->setColour(this->app->theme()->FG());
        this->addElement(this->heading);
        this->subLength = new Aether::Text(this->x() + 885, this->y() + 80, "", 20);
        this->subLength->setColour(this->app->theme()->muted());
        this->addElement(this->subLength);
        this->subTotal = new Aether::Text(this->x() + 885, this->subLength->y() - 25, "", 20);
        this->subTotal->setColour(this->app->theme()->muted());
        this->addElement(this->subTotal);
        this->titleH = new Aether::Text(this->x() + 65, this->y() + 150, "Title", 20);
        this->titleH->setColour(this->app->theme()->muted());
        this->addElement(this->titleH);
        this->artistH = new Aether::Text(this->x() + 425, this->y() + 150, "Artist", 20);
        this->artistH->setColour(this->app->theme()->muted());
        this->addElement(this->artistH);
        this->albumH = new Aether::Text(this->x() + 620, this->y() + 150, "Album", 20);
        this->albumH->setColour(this->app->theme()->muted());
        this->addElement(this->albumH);
        this->lengthH = new Aether::Text(this->x() + 850, this->y() + 150, "Length", 20);
        this->lengthH->setX(this->lengthH->x() - this->lengthH->w());
        this->lengthH->setColour(this->app->theme()->muted());
        this->addElement(this->lengthH);
        this->list = new Aether::List(this->x(), this->y() + 180, this->w() - 10, this->h() - 180);
        this->list->setScrollBarColour(this->app->theme()->muted2());
        this->list->setShowScrollBar(true);
        this->addElement(this->list);
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