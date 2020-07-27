#include "Application.hpp"
#include "ui/element/listitem/Song.hpp"
#include "ui/frame/Playlist.hpp"
#include "ui/overlay/ItemMenu.hpp"
#include "utils/Utils.hpp"

// Play button dimensions
#define BUTTON_F 26
#define BUTTON_W 150
#define BUTTON_H 50

// Size of Playlist image
#define IMAGE_SIZE 160

namespace Frame {
    Playlist::Playlist(Main::Application * app, PlaylistID id) : Frame(app) {
        // Reposition elements
        this->albumH->setY(this->albumH->y() + 80);
        this->titleH->setY(this->albumH->y());
        this->artistH->setY(this->albumH->y());
        this->lengthH->setY(this->albumH->y());
        this->list->setY(this->list->y() + 80);
        this->list->setH(this->list->h() - 80);

        // First get metadata for the provided playlist
        this->metadata = this->app->database()->getPlaylistMetadataForID(id);
        if (this->metadata.ID < 0) {
            // Helps show there was an error (should never appear)
            this->heading->setString("Playlist");
            return;
        }

        // Populate with Playlist's data
        Aether::Image * image = new Aether::Image(this->x() + 50, this->y() + 50, this->metadata.imagePath.empty() ? "romfs:/misc/noplaylist.png" : this->metadata.imagePath);
        image->setWH(IMAGE_SIZE, IMAGE_SIZE);
        this->addElement(image);
        this->heading->setString(this->metadata.name);
        this->heading->setX(image->x() + image->w() + 28);
        this->heading->setY(image->y() - 10);
        int maxW = (1280 - this->heading->x() - 30);
        if (this->heading->w() > maxW) {
            Aether::Text * tmp = new Aether::Text(this->heading->x() + maxW, this->heading->y(), "...", 60);
            tmp->setX(tmp->x() - tmp->w());
            this->addElement(tmp);
            this->heading->setW(maxW - tmp->w());
        }

        // Play and 'more' buttons
        Aether::FilledButton * playButton = new Aether::FilledButton(this->heading->x(), this->heading->y() + this->heading->h() + 45, BUTTON_W, BUTTON_H, "Play", BUTTON_F, [this]() {
            this->playPlaylist(0);
        });
        playButton->setFillColour(this->app->theme()->accent());
        playButton->setTextColour(Aether::Colour{0, 0, 0, 255});

        Aether::BorderButton * moreButton = new Aether::BorderButton(playButton->x() + playButton->w() + 20, playButton->y(), BUTTON_H, BUTTON_H, 2, "", BUTTON_F, [this]() {
            this->createPlaylistMenu();
        });
        moreButton->setBorderColour(this->app->theme()->FG());
        moreButton->setTextColour(this->app->theme()->FG());
        Aether::Image * dots = new Aether::Image(moreButton->x() + moreButton->w()/2, moreButton->y() + moreButton->h()/2, "romfs:/icons/verticaldots.png");
        dots->setXY(dots->x() - dots->w()/2, dots->y() - dots->h()/2);
        dots->setColour(this->app->theme()->FG());
        moreButton->addElement(dots);

        Aether::Container * c = new Aether::Container(playButton->x(), playButton->y(), moreButton->x() + moreButton->w() - playButton->x(), playButton->h());
        c->addElement(playButton);
        c->addElement(moreButton);
        this->addElement(c);

        this->emptyMsg = nullptr;
        this->goBack = false;
        this->msgbox = nullptr;
        this->playlistMenu = nullptr;
        this->songMenu = nullptr;

        // Populate list
        this->refreshList();
        this->setFocused(playButton);
    }

    void Playlist::playPlaylist(size_t pos) {
        if (this->songs.size() > 0) {
            std::vector<SongID> ids;
            for (size_t i = 0; i < this->songs.size(); i++) {
                ids.push_back(this->songs[i].ID);
            }
            this->app->sysmodule()->sendSetPlayingFrom(this->metadata.name);
            this->app->sysmodule()->sendSetQueue(ids);
            this->app->sysmodule()->sendSetSongIdx(pos);
            this->app->sysmodule()->sendSetShuffle(this->app->sysmodule()->shuffleMode());
        }
    }

    void Playlist::refreshList() {
        // Create list elements for each song
        this->list->removeAllElements();
        this->removeElement(this->emptyMsg);
        this->emptyMsg = nullptr;
        unsigned int totalSecs = 0;
        this->songs = this->app->database()->getSongMetadataForPlaylist(this->metadata.ID);
        if (this->songs.size() > 0) {
            for (size_t i = 0; i < this->songs.size(); i++) {
                totalSecs += this->songs[i].duration;
                CustomElm::ListItem::Song * l = new CustomElm::ListItem::Song();
                l->setTitleString(this->songs[i].title);
                l->setArtistString(this->songs[i].artist);
                l->setAlbumString(this->songs[i].album);
                l->setLengthString(Utils::secondsToHMS(this->songs[i].duration));
                l->setLineColour(this->app->theme()->muted2());
                l->setMoreColour(this->app->theme()->muted());
                l->setTextColour(this->app->theme()->FG());
                l->setCallback([this, i](){
                    this->playPlaylist(i);
                });
                l->setMoreCallback([this, i]() {
                    this->createSongMenu(i);
                });
                this->list->addElement(l);

                if (i == 0) {
                    l->setY(this->list->y() + 10);
                }
            }

            this->setFocussed(this->list);

        // Show message if no songs
        } else {
            this->list->setHidden(true);
            this->subLength->setHidden(true);
            this->emptyMsg = new Aether::Text(0, this->list->y() + this->list->h()*0.4, "This playlist contains no songs!", 24);
            this->emptyMsg->setColour(this->app->theme()->FG());
            this->emptyMsg->setX(this->x() + (this->w() - emptyMsg->w())/2);
            this->addElement(this->emptyMsg);
        }

        // Show number of songs and duration
        std::string str = std::to_string(this->metadata.songCount) + (this->metadata.songCount == 1 ? " song" : " songs");
        str += " | " + Utils::secondsToHoursMins(totalSecs);
        this->subTotal->setString(str);
        this->subTotal->setXY(this->heading->x() + 2, this->heading->y() + this->heading->h());
    }

    void Playlist::createDeleteMenu() {
        delete this->msgbox;
        this->msgbox = new Aether::MessageBox();
        this->msgbox->addLeftButton("Cancel", [this]() {
            // Do nothing; just close this overlay
            this->msgbox->close();
        });
        this->msgbox->addRightButton("Delete", [this]() {
            // Delete and update list
            this->app->lockDatabase();
            this->app->database()->removePlaylist(this->metadata.ID);
            this->app->unlockDatabase();
            this->goBack = true;
        });
        this->msgbox->setLineColour(this->app->theme()->muted2());
        this->msgbox->setRectangleColour(this->app->theme()->popupBG());
        this->msgbox->setTextColour(this->app->theme()->accent());
        Aether::Element * body = new Aether::Element(0, 0, 700);
        Aether::TextBlock * tips = new Aether::TextBlock(40, 40, "", 24, 620);
        tips->setString("Are you sure you want to delete this playlist? This action cannot be undone!");
        tips->setColour(this->app->theme()->FG());
        body->addElement(tips);
        body->setH(tips->h() + 80);
        this->msgbox->setBody(body);
        this->msgbox->setBodySize(body->w(), body->h());
        this->app->addOverlay(this->msgbox);
    }

    void Playlist::createPlaylistMenu() {
        // Create if doesn't exist
        if (this->playlistMenu == nullptr) {
            this->playlistMenu = new CustomOvl::Menu();
            this->playlistMenu->setBackgroundColour(this->app->theme()->popupBG());

            // Play Playlist
            CustomElm::MenuButton * b = new CustomElm::MenuButton();
            b->setIcon(new Aether::Image(0, 0, "romfs:/icons/playsmall.png"));
            b->setIconColour(this->app->theme()->muted());
            b->setText("Play");
            b->setTextColour(this->app->theme()->FG());
            b->setCallback([this]() {
                this->playPlaylist(0);
                this->playlistMenu->close();
            });
            this->playlistMenu->addButton(b);
            this->playlistMenu->addSeparator(this->app->theme()->muted2());

            // Add to Queue
            b = new CustomElm::MenuButton();
            b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoqueue.png"));
            b->setIconColour(this->app->theme()->muted());
            b->setText("Add to Queue");
            b->setTextColour(this->app->theme()->FG());
            b->setCallback([this]() {
                std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForPlaylist(this->metadata.ID);
                for (size_t i = 0; i < v.size(); i++) {
                    this->app->sysmodule()->sendAddToSubQueue(v[i].ID);
                }
                this->playlistMenu->close();
            });
            this->playlistMenu->addButton(b);

            // Add to other Playlist
            b = new CustomElm::MenuButton();
            b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoplaylist.png"));
            b->setIconColour(this->app->theme()->muted());
            b->setText("Add to other Playlist");
            b->setTextColour(this->app->theme()->FG());
            b->setCallback([this]() {
                this->showAddToPlaylist([this](PlaylistID i) {
                    if (i >= 0) {
                        for (size_t j = 0; j < this->songs.size(); j++) {
                            this->app->database()->addSongToPlaylist(i, this->songs[j].ID);
                        }
                        this->playlistMenu->close();
                        this->refreshList();
                    }
                });
            });
            this->playlistMenu->addButton(b);
            this->playlistMenu->addSeparator(this->app->theme()->muted2());

            // Delete playlist
            b = new CustomElm::MenuButton();
            b->setIcon(new Aether::Image(0, 0, "romfs:/icons/bin.png"));
            b->setIconColour(this->app->theme()->muted());
            b->setText("Delete Playlist");
            b->setTextColour(this->app->theme()->FG());
            b->setCallback([this]() {
                this->createDeleteMenu();
            });
            this->playlistMenu->addButton(b);
            this->playlistMenu->addSeparator(this->app->theme()->muted2());

            // View Information
            b = new CustomElm::MenuButton();
            b->setIcon(new Aether::Image(0, 0, "romfs:/icons/info.png"));
            b->setIconColour(this->app->theme()->muted());
            b->setText("View Information");
            b->setTextColour(this->app->theme()->FG());
            b->setCallback([this]() {
                // this->changeFrame(Type::PlaylistInfo, Action::Push, this->metadata.ID);
                // this->playlistMenu->close();
            });
            this->playlistMenu->addButton(b);

            // Finalize the menu
            this->playlistMenu->addButton(nullptr);
        }

        this->playlistMenu->resetHighlight();
        this->app->addOverlay(this->playlistMenu);
    }

    void Playlist::createSongMenu(size_t pos) {
        // Create menu
        delete this->songMenu;
        this->songMenu = new CustomOvl::ItemMenu();
        this->songMenu->setBackgroundColour(this->app->theme()->popupBG());
        this->songMenu->setMainTextColour(this->app->theme()->FG());
        this->songMenu->setSubTextColour(this->app->theme()->muted());
        this->songMenu->addSeparator(this->app->theme()->muted2());

        // Song metadata
        this->songMenu->setMainText(this->songs[pos].title);
        this->songMenu->setSubText(this->songs[pos].artist);
        this->songMenu->setImage(new Aether::Image(0, 0, this->metadata.imagePath.empty() ? "romfs:/misc/noalbum.png" : this->metadata.imagePath));

        // Play
        CustomElm::MenuButton * b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/playsmall.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Play");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, pos]() {
            this->playPlaylist(pos);
            this->songMenu->close();
        });
        this->songMenu->addButton(b);
        this->songMenu->addSeparator(this->app->theme()->muted2());

        // Add to Queue
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoqueue.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Add to Queue");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, pos]() {
            this->app->sysmodule()->sendAddToSubQueue(this->songs[pos].ID);
            this->songMenu->close();
        });
        this->songMenu->addButton(b);

        // Add to Playlist
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoplaylist.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Add to other Playlist");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, pos]() {
            this->showAddToPlaylist([this, pos](PlaylistID i) {
                if (i >= 0) {
                    this->app->database()->addSongToPlaylist(i, this->songs[pos].ID);
                    this->songMenu->close();
                }
            });
        });
        this->songMenu->addButton(b);
        this->songMenu->addSeparator(this->app->theme()->muted2());

        // Go to Artist
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/user.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Go to Artist");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, pos]() {
            ArtistID a = this->app->database()->getArtistIDForSong(this->songs[pos].ID);
            if (a >= 0) {
                this->changeFrame(Type::Artist, Action::Push, a);
            }
            this->songMenu->close();
        });
        this->songMenu->addButton(b);
        this->songMenu->addSeparator(this->app->theme()->muted2());

        // View Information
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/info.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("View Information");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, pos]() {
            // this->changeFrame(Type::SongInfo, Action::Push, this->songs[pos]);
        });
        this->songMenu->addButton(b);

        // Finalize the menu
        this->songMenu->addButton(nullptr);
        this->app->addOverlay(this->songMenu);
    }

    void Playlist::update(uint32_t dt) {
        // Update children first
        Frame::update(dt);

        // Change frame if flag set (only set if both the overlays are open)
        if (this->goBack) {
            this->msgbox->close();
            this->playlistMenu->close();
            this->changeFrame(Type::Playlists, Action::Back, 0);
        }
    }

    Playlist::~Playlist() {
        delete this->msgbox;
        delete this->playlistMenu;
        delete this->songMenu;
    }
};