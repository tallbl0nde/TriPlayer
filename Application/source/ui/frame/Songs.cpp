#include "Application.hpp"
#include "ui/element/listitem/Song.hpp"
#include "ui/frame/Songs.hpp"
#include "ui/overlay/ItemMenu.hpp"
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
                CustomElm::ListItem::Song * l = new CustomElm::ListItem::Song();
                l->setTitleString(m[i].title);
                l->setArtistString(m[i].artist);
                l->setAlbumString(m[i].album);
                l->setLengthString(Utils::secondsToHMS(m[i].duration));
                l->setLineColour(this->app->theme()->muted2());
                l->setMoreColour(this->app->theme()->muted());
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
        // Create menu
        delete this->menu;
        this->menu = new CustomOvl::ItemMenu();
        this->menu->setBackgroundColour(this->app->theme()->popupBG());
        this->menu->setMainTextColour(this->app->theme()->FG());
        this->menu->setSubTextColour(this->app->theme()->muted());
        this->menu->addSeparator(this->app->theme()->muted2());

        // Song metadata
        Metadata::Song m = this->app->database()->getSongMetadataForID(id);
        this->menu->setMainText(m.title);
        this->menu->setSubText(m.artist);
        AlbumID aID = this->app->database()->getAlbumIDForSong(m.ID);
        Metadata::Album md = this->app->database()->getAlbumMetadataForID(aID);
        this->menu->setImage(new Aether::Image(0, 0, md.imagePath.empty() ? "romfs:/misc/noalbum.png" : md.imagePath));

        // Add to Queue
        CustomElm::MenuButton * b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoqueue.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Add to Queue");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, id]() {
            this->app->sysmodule()->sendAddToSubQueue(id);
            this->menu->close();
        });
        this->menu->addButton(b);

        // Add to Playlist
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoplaylist.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Add to Playlist");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, id]() {
            this->showAddToPlaylist([this, id](PlaylistID i) {
                if (i >= 0) {
                    this->app->database()->addSongToPlaylist(i, id);
                    this->menu->close();
                }
            });
        });
        this->menu->addButton(b);
        this->menu->addSeparator(this->app->theme()->muted2());

        // Go to Artist
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/user.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Go to Artist");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, id]() {
            ArtistID a = this->app->database()->getArtistIDForSong(id);
            if (a >= 0) {
                this->changeFrame(Type::Artist, Action::Push, a);
            }
            this->menu->close();
        });
        this->menu->addButton(b);

        // Go to Album
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/disc.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Go to Album");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, id]() {
            AlbumID a = this->app->database()->getAlbumIDForSong(id);
            if (a >= 0) {
                this->changeFrame(Type::Album, Action::Push, a);
            }
            this->menu->close();
        });
        this->menu->addButton(b);
        this->menu->addSeparator(this->app->theme()->muted2());

        // View Information
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/info.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("View Information");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, id]() {
            // this->changeFrame(Type::SongInfo, Action::Push, id);
        });
        this->menu->addButton(b);

        // Finalize the menu
        this->menu->addButton(nullptr);
        this->app->addOverlay(this->menu);
    }

    Songs::~Songs() {
        delete this->menu;
    }
};