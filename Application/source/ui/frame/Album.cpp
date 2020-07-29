#include "Application.hpp"
#include "ui/element/listitem/AlbumSong.hpp"
#include "ui/frame/Album.hpp"
#include "ui/overlay/ArtistList.hpp"
#include "ui/overlay/ItemMenu.hpp"
#include "utils/Utils.hpp"

// Play button dimensions
#define BUTTON_F 26
#define BUTTON_W 150
#define BUTTON_H 50

// Size of album image
#define IMAGE_SIZE 160

namespace Frame {
    Album::Album(Main::Application * app, AlbumID id) : Frame(app) {
        // Reposition elements
        this->albumH->setString("#");
        this->albumH->setXY(this->titleH->x(), this->albumH->y() + 80);
        this->titleH->setXY(this->x() + 118, this->albumH->y());
        this->artistH->setXY(this->x() + 562, this->albumH->y());
        this->lengthH->setY(this->albumH->y());
        this->list->setY(this->list->y() + 80);
        this->list->setH(this->list->h() - 80);

        // First get metadata for the provided album
        this->metadata = this->app->database()->getAlbumMetadataForID(id);
        if (this->metadata.ID < 0) {
            // Helps show there was an error (should never appear)
            this->heading->setString("Album");
            return;
        }
        this->oneArtist = (this->metadata.artist != "Various Artists");
        if (this->oneArtist) {
            this->artistH->setHidden(true);
        }

        // Populate with Album's data
        Aether::Image * image = new Aether::Image(this->x() + 50, this->y() + 50, this->metadata.imagePath.empty() ? "romfs:/misc/noalbum.png" : this->metadata.imagePath);
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

        std::string str = this->metadata.artist;
        str += " | " + std::to_string(this->metadata.songCount) + (this->metadata.songCount == 1 ? " song" : " songs");
        this->subTotal->setString(str);
        this->subTotal->setXY(this->heading->x() + 2, this->heading->y() + this->heading->h());

        // Play and 'more' buttons
        Aether::FilledButton * playButton = new Aether::FilledButton(this->subTotal->x(), this->subTotal->y() + this->subTotal->h() + 20, BUTTON_W, BUTTON_H, "Play", BUTTON_F, [this]() {
            this->playAlbum(0);
        });
        playButton->setFillColour(this->app->theme()->accent());
        playButton->setTextColour(Aether::Colour{0, 0, 0, 255});

        Aether::BorderButton * moreButton = new Aether::BorderButton(playButton->x() + playButton->w() + 20, playButton->y(), BUTTON_H, BUTTON_H, 2, "", BUTTON_F, [this]() {
            this->createAlbumMenu();
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

        // Get all the songs in the album (ordered by disc, track number and finally alphabetically)
        this->songs = this->app->database()->getSongMetadataForAlbum(this->metadata.ID);

        // Create list elements for each song
        if (this->songs.size() > 0) {
            int lastDisc = -1;
            for (size_t i = 0; i < this->songs.size(); i++) {
                // Insert a heading prior to element if different disc
                if (this->songs[i].discNumber != lastDisc) {
                    std::string str = "";
                    if (this->songs[i].discNumber == 0) {
                        // Add a spacer if not a disc
                        if (lastDisc != -1) {
                            this->list->addElement(new Aether::Element(0, 0, 100, 60));
                        }
                    } else {
                        // Otherwise add "Disc x"
                        Aether::Element * e = new Aether::Element(0, 0, 100, 70);
                        Aether::Text * t = new Aether::Text(0, 0, "Disc " + std::to_string(this->songs[i].discNumber), 28);
                        t->setColour(this->app->theme()->FG());
                        t->setY(e->h() - t->h() - 10);
                        e->addElement(t);
                        this->list->addElement(e);
                        if (lastDisc == -1) {
                            e->setY(this->list->y() - 20);
                        }
                    }
                    lastDisc = this->songs[i].discNumber;
                }

                CustomElm::ListItem::AlbumSong * l = new CustomElm::ListItem::AlbumSong();
                if (this->songs[i].trackNumber > 0) {
                    l->setTrackString(std::to_string(this->songs[i].trackNumber));
                }
                l->setTitleString(this->songs[i].title);
                if (!this->oneArtist) {
                    l->setArtistString(this->songs[i].artist);
                }
                l->setLengthString(Utils::secondsToHMS(this->songs[i].duration));
                l->setLineColour(this->app->theme()->muted2());
                l->setMoreColour(this->app->theme()->muted());
                l->setTextColour(this->app->theme()->FG());
                l->setCallback([this, i]() {
                    this->playAlbum(i);
                });
                l->setMoreCallback([this, i]() {
                    this->createSongMenu(i);
                });
                this->list->addElement(l);
                if (i == 0 && lastDisc == 0) {
                    l->setY(this->list->y() + 10);
                }
            }
        }

        this->artistsList = nullptr;
        this->albumMenu = nullptr;
        this->songMenu = nullptr;
    }

    void Album::playAlbum(size_t pos) {
        std::vector<SongID> ids;
        for (size_t i = 0; i < this->songs.size(); i++) {
            ids.push_back(this->songs[i].ID);
        }
        this->app->sysmodule()->sendSetPlayingFrom(this->metadata.name);
        this->app->sysmodule()->sendSetQueue(ids);
        this->app->sysmodule()->sendSetSongIdx(pos);
        this->app->sysmodule()->sendSetShuffle(this->app->sysmodule()->shuffleMode());
    }

    void Album::createArtistsList() {
        // Query database for artists first
        std::vector<Metadata::Artist> m = this->app->database()->getArtistMetadataForAlbum(this->metadata.ID);

        // Create menu
        delete this->artistsList;
        this->artistsList = new CustomOvl::ArtistList();
        this->artistsList->setBackgroundColour(this->app->theme()->popupBG());

        // Populate with artists
        for (size_t i = 0; i < m.size(); i++) {
            CustomElm::ListItem::Artist * l = new CustomElm::ListItem::Artist(m[i].imagePath.empty() ? "romfs:/misc/noartist.png" : m[i].imagePath);
            l->setName(m[i].name);
            l->setLineColour(this->app->theme()->muted2());
            l->setTextColour(this->app->theme()->FG());
            ArtistID aID = m[i].ID;
            l->setCallback([this, aID]() {
                this->changeFrame(Type::Artist, Action::Push, aID);
                this->artistsList->close();
            });
            this->artistsList->addArtist(l);
        }

        this->app->addOverlay(this->artistsList);
    }

    void Album::createAlbumMenu() {
        // Create if doesn't exist
        if (this->albumMenu == nullptr) {
            this->albumMenu = new CustomOvl::Menu();
            this->albumMenu->setBackgroundColour(this->app->theme()->popupBG());

            // Add to Queue
            CustomElm::MenuButton * b = new CustomElm::MenuButton();
            b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoqueue.png"));
            b->setIconColour(this->app->theme()->muted());
            b->setText("Add to Queue");
            b->setTextColour(this->app->theme()->FG());
            b->setCallback([this]() {
                std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForAlbum(this->metadata.ID);
                for (size_t i = 0; i < v.size(); i++) {
                    this->app->sysmodule()->sendAddToSubQueue(v[i].ID);
                }
                this->albumMenu->close();
            });
            this->albumMenu->addButton(b);

            // Add to Playlist
            b = new CustomElm::MenuButton();
            b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoplaylist.png"));
            b->setIconColour(this->app->theme()->muted());
            b->setText("Add to Playlist");
            b->setTextColour(this->app->theme()->FG());
            b->setCallback([this]() {
                this->showAddToPlaylist([this](PlaylistID i) {
                    if (i >= 0) {
                        std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForAlbum(this->metadata.ID);
                        for (size_t j = 0; j < v.size(); j++) {
                            this->app->database()->addSongToPlaylist(i, v[j].ID);
                        }
                        this->albumMenu->close();
                    }
                });
            });
            this->albumMenu->addButton(b);
            this->albumMenu->addSeparator(this->app->theme()->muted2());

            // View Artist(s)
            b = new CustomElm::MenuButton();
            b->setIcon(new Aether::Image(0, 0, "romfs:/icons/user.png"));
            b->setIconColour(this->app->theme()->muted());
            b->setTextColour(this->app->theme()->FG());
            if (this->oneArtist) {
                b->setText("Go to Artist");
                b->setCallback([this]() {
                    ArtistID aID = this->app->database()->getArtistIDForName(this->metadata.artist);
                    this->changeFrame(Type::Artist, Action::Push, aID);
                    this->albumMenu->close();
                });

            } else {
                b->setText("View Artists");
                b->setCallback([this]() {
                    this->createArtistsList();
                    this->albumMenu->close();
                });
            }
            this->albumMenu->addButton(b);
            this->albumMenu->addSeparator(this->app->theme()->muted2());

            // View Information
            b = new CustomElm::MenuButton();
            b->setIcon(new Aether::Image(0, 0, "romfs:/icons/info.png"));
            b->setIconColour(this->app->theme()->muted());
            b->setText("View Information");
            b->setTextColour(this->app->theme()->FG());
            b->setCallback([this]() {
                this->changeFrame(Type::AlbumInfo, Action::Push, this->metadata.ID);
                this->albumMenu->close();
            });
            this->albumMenu->addButton(b);

            // Finalize the menu
            this->albumMenu->addButton(nullptr);
        }

        this->albumMenu->resetHighlight();
        this->app->addOverlay(this->albumMenu);
    }

    void Album::createSongMenu(size_t pos) {
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

        // Add to Queue
        CustomElm::MenuButton * b = new CustomElm::MenuButton();
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
        b->setText("Add to Playlist");
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

        // Go to Artist (if multiple artists)
        if (!this->oneArtist) {
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
        }

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

    Album::~Album() {
        delete this->artistsList;
        delete this->albumMenu;
        delete this->songMenu;
    }
};