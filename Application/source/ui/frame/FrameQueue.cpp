#include "Application.hpp"
#include "FrameQueue.hpp"
#include "ListSong.hpp"
#include "Utils.hpp"

namespace Frame {
    Queue::Queue(Main::Application * a) : Frame(a) {
        this->heading->setString("Play Queue");

        // Create items for songs in queue
        unsigned int totalSecs = 0;
        this->songIDs = this->app->sysmodule()->queue();
        size_t currentSong = this->app->sysmodule()->songIdx();
        for (size_t i = 0; i < this->songIDs.size(); i++) {
            SongInfo si = this->app->database()->getSongInfoForID(this->songIDs[i]);
            totalSecs += si.duration;
            CustomElm::ListSong * l = new CustomElm::ListSong();
            l->setTitleString(si.title);
            l->setArtistString(si.artist);
            l->setAlbumString(si.album);
            l->setLengthString(Utils::secondsToHMS(si.duration));
            l->setLineColour(this->app->theme()->muted2());
            l->setTextColour(currentSong == i ? this->app->theme()->accent() : this->app->theme()->FG());
            l->setCallback([this, i](){
                this->app->sysmodule()->sendSetSongIdx(i);
            });
            this->list->addElement(l);

            if (i == 0) {
                l->setY(this->list->y() + 10);
            }
        }

        this->subLength->setString(Utils::secondsToHoursMins(totalSecs));
        this->subLength->setX(1205 - this->subLength->w());
        this->subTotal->setString(std::to_string(this->songIDs.size()) + (this->songIDs.size() == 1 ? " track" : " tracks" ));
        this->subTotal->setX(1205 - this->subTotal->w());

        this->setFocussed(this->list);
    }
};