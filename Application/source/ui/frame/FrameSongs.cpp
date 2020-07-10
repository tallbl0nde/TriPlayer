#include "Application.hpp"
#include "FrameSongs.hpp"
#include "ListSong.hpp"
#include "Utils.hpp"

#include "SongMenu.hpp"

namespace Frame {
    Songs::Songs(Main::Application * a) : Frame(a) {
        this->heading->setString("Songs");

        // Create items for songs
        unsigned int totalSecs = 0;
        std::vector<SongInfo> si = this->app->database()->getAllSongInfo();
        if (si.size() > 0) {
            for (size_t i = 0; i < si.size(); i++) {
                this->songIDs.push_back(si[i].ID);
                totalSecs += si[i].duration;
                CustomElm::ListSong * l = new CustomElm::ListSong();
                l->setTitleString(si[i].title);
                l->setArtistString(si[i].artist);
                l->setAlbumString(si[i].album);
                l->setLengthString(Utils::secondsToHMS(si[i].duration));
                l->setDotsColour(this->app->theme()->muted());
                l->setLineColour(this->app->theme()->muted2());
                l->setTextColour(this->app->theme()->FG());
                l->setCallback([this, i](){
                    this->app->sysmodule()->sendSetQueue(this->songIDs);
                    this->app->sysmodule()->sendSetSongIdx(i);
                    this->app->sysmodule()->sendSetShuffle(this->app->sysmodule()->shuffleMode());
                });
                SongID id = si[i].ID;
                l->setMoreCallback([this, id]() {
                    this->app->showSongMenu(id);
                });
                this->list->addElement(l);

                if (i == 0) {
                    l->setY(this->list->y() + 10);
                }
            }

            this->subLength->setString(Utils::secondsToHoursMins(totalSecs));
            this->subLength->setX(this->x() + 885 - this->subLength->w());
            this->subTotal->setString(std::to_string(si.size()) + (si.size() == 1 ? " track" : " tracks" ));
            this->subTotal->setX(this->x() + 885 - this->subTotal->w());

            this->setFocussed(this->list);

        // Show message if no songs
        } else {
            this->list->setHidden(true);
            this->subLength->setHidden(true);
            this->subTotal->setHidden(true);
            Aether::Text * emptyMsg = new Aether::Text(0, this->list->y() + this->list->h()*0.4, "No songs found in /music!", 24);
            emptyMsg->setColour(this->app->theme()->FG());
            emptyMsg->setX(this->x() + (this->w() - emptyMsg->w())/2);
            this->addElement(emptyMsg);
        }
    }
};