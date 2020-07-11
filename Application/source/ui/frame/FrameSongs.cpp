#include "Application.hpp"
#include "ui/element/ListSong.hpp"
#include "ui/frame/FrameSongs.hpp"
#include "ui/overlay/SongMenu.hpp"
#include "utils/Utils.hpp"

namespace Frame {
    Songs::Songs(Main::Application * a) : Frame(a) {
        this->heading->setString("Songs");

        // Create items for songs
        unsigned int totalSecs = 0;
        std::vector<Metadata::Song> m = this->app->database()->getAllSongMetadata();
        if (m.size() > 0) {
            for (size_t i = 0; i < m.size(); i++) {
                this->songIDs.push_back(m[i].ID);
                totalSecs += m[i].duration;
                CustomElm::ListSong * l = new CustomElm::ListSong();
                l->setTitleString(m[i].title);
                l->setArtistString(m[i].artist);
                l->setAlbumString(m[i].album);
                l->setLengthString(Utils::secondsToHMS(m[i].duration));
                l->setDotsColour(this->app->theme()->muted());
                l->setLineColour(this->app->theme()->muted2());
                l->setTextColour(this->app->theme()->FG());
                l->setCallback([this, i](){
                    this->app->sysmodule()->sendSetPlayingFrom("Your Songs");
                    this->app->sysmodule()->sendSetQueue(this->songIDs);
                    this->app->sysmodule()->sendSetSongIdx(i);
                    this->app->sysmodule()->sendSetShuffle(this->app->sysmodule()->shuffleMode());
                });
                SongID id = m[i].ID;
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
            this->subTotal->setString(std::to_string(m.size()) + (m.size() == 1 ? " track" : " tracks" ));
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