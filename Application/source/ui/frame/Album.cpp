#include "Application.hpp"
#include "ui/element/ListAlbumSong.hpp"
#include "ui/frame/Album.hpp"
#include "ui/overlay/menu/Album.hpp"
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
        Metadata::Album m = this->app->database()->getAlbumMetadataForID(id);
        if (m.ID < 0) {
            // Helps show there was an error (should never appear)
            this->heading->setString("Album");
            return;
        }
        bool oneArtist = (m.artist != "Various Artists");
        if (oneArtist) {
            this->artistH->setHidden(true);
        }

        // Populate with Album's data
        Aether::Image * image = new Aether::Image(this->x() + 50, this->y() + 50, m.imagePath.empty() ? "romfs:/misc/noalbum.png" : m.imagePath);
        image->setWH(IMAGE_SIZE, IMAGE_SIZE);
        this->addElement(image);
        this->heading->setString(m.name);
        this->heading->setX(image->x() + image->w() + 28);
        this->heading->setY(image->y() - 10);
        int maxW = (1280 - this->heading->x() - 30);
        if (this->heading->w() > maxW) {
            Aether::Text * tmp = new Aether::Text(this->heading->x() + maxW, this->heading->y(), "...", 60);
            tmp->setX(tmp->x() - tmp->w());
            this->addElement(tmp);
            this->heading->setW(maxW - tmp->w());
        }

        std::string str = m.artist;
        str += " | " + std::to_string(m.songCount) + (m.songCount == 1 ? " song" : " songs");
        this->subTotal->setString(str);
        this->subTotal->setXY(this->heading->x() + 2, this->heading->y() + this->heading->h());

        // Play and 'more' buttons
        Aether::FilledButton * playButton = new Aether::FilledButton(this->subTotal->x(), this->subTotal->y() + this->subTotal->h() + 20, BUTTON_W, BUTTON_H, "Play", BUTTON_F, [this, m]() {
            this->playAlbum(m);
        });
        playButton->setFillColour(this->app->theme()->accent());
        playButton->setTextColour(Aether::Colour{0, 0, 0, 255});

        Aether::BorderButton * moreButton = new Aether::BorderButton(playButton->x() + playButton->w() + 20, playButton->y(), BUTTON_H, BUTTON_H, 2, "", BUTTON_F, [this, id]() {
            this->createAlbumMenu(id);
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
        std::vector<Metadata::Song> md = this->app->database()->getSongMetadataForAlbum(m.ID);

        // Create list elements for each song
        if (md.size() > 0) {
            int lastDisc = -1;
            for (size_t i = 0; i < md.size(); i++) {
                // Insert a heading prior to element if different disc
                if (md[i].discNumber != lastDisc) {
                    std::string str = "";
                    if (md[i].discNumber == 0) {
                        // Add a spacer if not a disc
                        if (lastDisc != -1) {
                            this->list->addElement(new Aether::Element(0, 0, 100, 60));
                        }
                    } else {
                        // Otherwise add "Disc x"
                        Aether::Element * e = new Aether::Element(0, 0, 100, 70);
                        Aether::Text * t = new Aether::Text(0, 0, "Disc " + std::to_string(md[i].discNumber), 28);
                        t->setColour(this->app->theme()->FG());
                        t->setY(e->h() - t->h() - 10);
                        e->addElement(t);
                        this->list->addElement(e);
                        if (lastDisc == -1) {
                            e->setY(this->list->y() - 20);
                        }
                    }
                    lastDisc = md[i].discNumber;
                }

                CustomElm::ListAlbumSong * l = new CustomElm::ListAlbumSong();
                if (md[i].trackNumber > 0) {
                    l->setTrackString(std::to_string(md[i].trackNumber));
                }
                l->setTitleString(md[i].title);
                if (!oneArtist) {
                    l->setArtistString(md[i].artist);
                }
                l->setLengthString(Utils::secondsToHMS(md[i].duration));
                l->setDotsColour(this->app->theme()->muted());
                l->setLineColour(this->app->theme()->muted2());
                l->setTextColour(this->app->theme()->FG());
                l->setCallback([this, i](){
                    // this->app->sysmodule()->sendSetPlayingFrom("Your Songs");
                    // this->app->sysmodule()->sendSetQueue(this->songIDs);
                    // this->app->sysmodule()->sendSetSongIdx(i);
                    // this->app->sysmodule()->sendSetShuffle(this->app->sysmodule()->shuffleMode());
                });
                SongID id = md[i].ID;
                l->setMoreCallback([this, id]() {
                    // this->createMenu(id);
                });
                this->list->addElement(l);
                if (i == 0 && lastDisc == 0) {
                    l->setY(this->list->y() + 10);
                }
            }
        }

        this->menu = nullptr;
    }

    void Album::playAlbum(Metadata::Album m) {
        std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForAlbum(m.ID);
        std::vector<SongID> ids;
        for (size_t i = 0; i < v.size(); i++) {
            ids.push_back(v[i].ID);
        }
        this->app->sysmodule()->sendSetPlayingFrom(m.name);
        this->app->sysmodule()->sendSetQueue(ids);
        this->app->sysmodule()->sendSetSongIdx(0);
        this->app->sysmodule()->sendSetShuffle(this->app->sysmodule()->shuffleMode());
    }

    void Album::createAlbumMenu(AlbumID id) {
        // Don't create another menu if one exists
        if (this->menu == nullptr) {
            // Query database first
            Metadata::Album m = this->app->database()->getAlbumMetadataForID(id);
            if (m.ID < 0) {
                return;
            }

            bool oneArtist = (m.artist != "Various Artists");
            this->menu = new CustomOvl::Menu::Album(CustomOvl::Menu::Type::HideTop);
            this->menu->setPlayAllText("Play");
            this->menu->setAddToQueueText("Add to Queue");
            this->menu->setAddToPlaylistText("Add to Playlist");
            this->menu->setGoToArtistText(oneArtist ? "Go to Artist" : "View Artists");
            this->menu->setViewInformationText("View Information");
            this->menu->setBackgroundColour(this->app->theme()->popupBG());
            this->menu->setIconColour(this->app->theme()->muted());
            this->menu->setLineColour(this->app->theme()->muted2());
            this->menu->setMutedTextColour(this->app->theme()->muted());
            this->menu->setTextColour(this->app->theme()->FG());

            this->menu->setPlayAllFunc([this, m]() {
                this->playAlbum(m);
                this->menu->close();
            });

            this->menu->setAddToQueueFunc([this, m]() {
                std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForAlbum(m.ID);
                for (size_t i = 0; i < v.size(); i++) {
                    this->app->sysmodule()->sendAddToSubQueue(v[i].ID);
                }
                this->menu->close();
            });

            this->menu->setAddToPlaylistFunc(nullptr);

            // Set callback based on number of artists
            if (oneArtist) {
                ArtistID id = this->app->database()->getArtistIDForName(m.artist);
                this->menu->setGoToArtistFunc([this, id]() {
                    this->changeFrame(Type::Artist, Action::Push, id);
                    this->menu->close();
                });
            } else {
                this->menu->setGoToArtistFunc([this, m]() {
                    // this->createArtistsMenu(m.ID);
                    this->menu->close();
                });
            }

            this->menu->setViewInformationFunc([this, m]() {
                // this->changeFrame(Type::AlbumInfo, Action::Push, m.ID);
                // this->menu->close();
            });
        }

        this->menu->resetHighlight();
        this->app->addOverlay(this->menu);
    }

    Album::~Album() {
        delete this->menu;
    }
};