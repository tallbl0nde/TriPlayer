#include "Application.hpp"
#include "ui/element/ListSong.hpp"
#include "ui/frame/Songs.hpp"
#include "utils/MP3.hpp"
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
                    this->createMenu(id);
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

        this->menu = nullptr;
    }

    void Songs::createMenu(SongID id) {
        // Create the base elements if they don't already exist
        if (this->menu == nullptr) {
            // Don't need to show the 'Remove from Queue' option here
            this->menu = new CustomOvl::Menu::Song(CustomOvl::Menu::Song::Type::HideRemove);
            this->menu->setAddToQueueText("Add to Queue");
            this->menu->setAddToPlaylistText("Add to Playlist");
            this->menu->setGoToArtistText("Go to Artist");
            this->menu->setGoToAlbumText("Go to Album");
            this->menu->setViewInformationText("View Information");
            this->menu->setBackgroundColour(this->app->theme()->popupBG());
            this->menu->setIconColour(this->app->theme()->muted());
            this->menu->setLineColour(this->app->theme()->muted2());
            this->menu->setMutedTextColour(this->app->theme()->muted());
            this->menu->setTextColour(this->app->theme()->FG());
        }

        // Song metadata
        Metadata::Song m = this->app->database()->getSongMetadataForID(id);
        this->menu->setTitle(m.title);
        this->menu->setArtist(m.artist);

        // Album art (this will likely be improved at some point)
        bool hasArt = false;
        if (m.path.length() > 0) {
            Metadata::Art art = Utils::MP3::getArtFromID3(m.path);
            if (art.data != nullptr) {
                this->menu->setAlbum(new Aether::Image(0, 0, art.data, art.size));
                hasArt = true;
                delete[] art.data;
            }
        }
        if (!hasArt) {
            this->menu->setAlbum(new Aether::Image(0, 0, "romfs:/misc/noalbum.png"));
        }

        // Callbacks
        this->menu->setAddToQueueFunc([this, id]() {
            this->app->sysmodule()->sendAddToSubQueue(id);
            this->menu->close();
        });
        this->menu->setAddToPlaylistFunc([this, id]() {
            // add to playlist
        });
        this->menu->setGoToArtistFunc([this, id]() {
            ArtistID a = this->app->database()->getArtistIDForSong(id);
            if (a >= 0) {
                this->changeFrame(Type::Artist, Action::Push, a);
            }
            this->menu->close();
        });
        this->menu->setGoToAlbumFunc([this, id]() {
            // go to album
        });
        this->menu->setViewInformationFunc([this, id]() {
            // view information
        });

        this->menu->resetHighlight();
        this->app->addOverlay(this->menu);
    }

    Songs::~Songs() {
        delete this->menu;
    }
};