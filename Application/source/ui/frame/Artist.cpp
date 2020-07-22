#include "Application.hpp"
#include "ui/element/GridItem.hpp"
#include "ui/element/ScrollableGrid.hpp"
#include "ui/frame/Artist.hpp"
#include "ui/overlay/menu/Artist.hpp"
#include "ui/overlay/menu/Album.hpp"

// Play button dimensions
#define BUTTON_F 26
#define BUTTON_W 150
#define BUTTON_H 50

// Size of artist image
#define IMAGE_SIZE 160

namespace Frame {
    Artist::Artist(Main::Application * app, ArtistID id) : Frame(app) {
        // Remove list + headings (I should redo Frame to avoid this)
        this->removeElement(this->list);
        this->removeElement(this->titleH);
        this->removeElement(this->artistH);
        this->removeElement(this->albumH);
        this->removeElement(this->lengthH);

        // First get metadata for the provided artist
        Metadata::Artist m = this->app->database()->getArtistMetadataForID(id);
        if (m.ID < 0) {
            // Helps show there was an error (should never appear)
            this->heading->setString("Artist");
            return;
        }

        // Populate with Artist's data
        Aether::Image * image = new Aether::Image(this->x() + 50, this->y() + 50, m.imagePath.empty() ? "romfs:/misc/noartist.png" : m.imagePath);
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

        std::string str = std::to_string(m.albumCount) + (m.albumCount == 1 ? " album" : " albums");
        str += " | " + std::to_string(m.songCount) + (m.songCount == 1 ? " song" : " songs");
        this->subTotal->setString(str);
        this->subTotal->setXY(this->heading->x() + 2, this->heading->y() + this->heading->h());

        // Play and 'more' buttons
        Aether::FilledButton * playButton = new Aether::FilledButton(this->subTotal->x(), this->subTotal->y() + this->subTotal->h() + 20, BUTTON_W, BUTTON_H, "Play", BUTTON_F, [this, m]() {
            std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForArtist(m.ID);
            std::vector<SongID> ids;
            for (size_t i = 0; i < v.size(); i++) {
                ids.push_back(v[i].ID);
            }
            this->app->sysmodule()->sendSetPlayingFrom(m.name);
            this->app->sysmodule()->sendSetQueue(ids);
            this->app->sysmodule()->sendSetSongIdx(0);
            this->app->sysmodule()->sendSetShuffle(this->app->sysmodule()->shuffleMode());
        });
        playButton->setFillColour(this->app->theme()->accent());
        playButton->setTextColour(Aether::Colour{0, 0, 0, 255});

        Aether::BorderButton * moreButton = new Aether::BorderButton(playButton->x() + playButton->w() + 20, playButton->y(), BUTTON_H, BUTTON_H, 2, "", BUTTON_F, [this, id]() {
            this->createArtistMenu(id);
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

        // Get a list of the artist's albums
        std::vector<Metadata::Album> md = this->app->database()->getAlbumMetadataForArtist(m.ID);

        // Create grid if there are albums
        if (md.size() > 0) {
            int gridY = image->y() + image->h() + 20;
            CustomElm::ScrollableGrid * grid = new CustomElm::ScrollableGrid(this->x(), gridY, this->w() - 10, this->h() - gridY, 250, 3);
            grid->setShowScrollBar(true);
            grid->setScrollBarColour(this->app->theme()->muted2());

            // Populate grid with albums
            for (size_t i = 0; i < md.size(); i++) {
                CustomElm::GridItem * l = new CustomElm::GridItem(md[i].imagePath.empty() ? "romfs:/misc/noalbum.png" : md[i].imagePath);
                l->setMainString(md[i].name);
                std::string str = std::to_string(md[i].songCount) + (md[i].songCount == 1 ? " song" : " songs");
                l->setSubString(str);
                l->setDotsColour(this->app->theme()->muted());
                l->setTextColour(this->app->theme()->FG());
                l->setMutedTextColour(this->app->theme()->muted());
                AlbumID id = md[i].ID;
                l->setCallback([this, id](){
                    // this->changeFrame(Type::Album, Action::Push, id);
                });
                l->setMoreCallback([this, id]() {
                    this->createAlbumMenu(id);
                });
                grid->addElement(l);
            }

            this->addElement(grid);
        }

        this->albumMenu = nullptr;
        this->menu = nullptr;
    }

    void Artist::createArtistMenu(ArtistID id) {
        // Don't create another menu if one exists
        if (this->menu == nullptr) {
            // Query database first
            Metadata::Artist m = this->app->database()->getArtistMetadataForID(id);
            if (m.ID < 0) {
                return;
            }

            this->menu = new CustomOvl::Menu::Artist(CustomOvl::Menu::Type::HideTop);
            this->menu->setPlayAllText("Play All");
            this->menu->setAddToQueueText("Add to Queue");
            this->menu->setAddToPlaylistText("Add to Playlist");
            this->menu->setViewInformationText("View Information");
            this->menu->setBackgroundColour(this->app->theme()->popupBG());
            this->menu->setIconColour(this->app->theme()->muted());
            this->menu->setLineColour(this->app->theme()->muted2());
            this->menu->setMutedTextColour(this->app->theme()->muted());
            this->menu->setTextColour(this->app->theme()->FG());

            this->menu->setPlayAllFunc([this, m]() {
                std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForArtist(m.ID);
                std::vector<SongID> ids;
                for (size_t i = 0; i < v.size(); i++) {
                    ids.push_back(v[i].ID);
                }
                this->app->sysmodule()->sendSetPlayingFrom(m.name);
                this->app->sysmodule()->sendSetQueue(ids);
                this->app->sysmodule()->sendSetSongIdx(0);
                this->app->sysmodule()->sendSetShuffle(this->app->sysmodule()->shuffleMode());
                this->menu->close();
            });

            this->menu->setAddToQueueFunc([this, m]() {
                std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForArtist(m.ID);
                for (size_t i = 0; i < v.size(); i++) {
                    this->app->sysmodule()->sendAddToSubQueue(v[i].ID);
                }
                this->menu->close();
            });
            this->menu->setAddToPlaylistFunc(nullptr);
            this->menu->setViewInformationFunc([this, m]() {
                this->changeFrame(Type::ArtistInfo, Action::Push, m.ID);
                this->menu->close();
            });
        }

        this->menu->resetHighlight();
        this->app->addOverlay(this->menu);
    }

    void Artist::createAlbumMenu(AlbumID id) {
        // Query database first
        Metadata::Album m = this->app->database()->getAlbumMetadataForID(id);
        if (m.ID < 0) {
            return;
        }

        // Don't create another menu if one exists
        if (this->albumMenu == nullptr) {
            // Query database first
            Metadata::Artist m = this->app->database()->getArtistMetadataForID(id);
            if (m.ID < 0) {
                return;
            }

            this->albumMenu = new CustomOvl::Menu::Artist(CustomOvl::Menu::Type::Normal);
            this->albumMenu->setPlayAllText("Play Album");
            this->albumMenu->setAddToQueueText("Add to Queue");
            this->albumMenu->setAddToPlaylistText("Add to Playlist");
            this->albumMenu->setViewInformationText("View Information");
            this->albumMenu->setBackgroundColour(this->app->theme()->popupBG());
            this->albumMenu->setIconColour(this->app->theme()->muted());
            this->albumMenu->setLineColour(this->app->theme()->muted2());
            this->albumMenu->setMutedTextColour(this->app->theme()->muted());
            this->albumMenu->setTextColour(this->app->theme()->FG());
        }

        // Set album specific things
        this->albumMenu->setImage(new Aether::Image(0, 0, m.imagePath.empty() ? "romfs:/misc/noalbum.png" : m.imagePath));
        this->albumMenu->setName(m.name);
        this->albumMenu->setStats(m.artist);

        this->albumMenu->setPlayAllFunc([this, m]() {
            std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForAlbum(m.ID);
            std::vector<SongID> ids;
            for (size_t i = 0; i < v.size(); i++) {
                ids.push_back(v[i].ID);
            }
            this->app->sysmodule()->sendSetPlayingFrom(m.name);
            this->app->sysmodule()->sendSetQueue(ids);
            this->app->sysmodule()->sendSetSongIdx(0);
            this->app->sysmodule()->sendSetShuffle(this->app->sysmodule()->shuffleMode());
            this->albumMenu->close();
        });

        this->albumMenu->setAddToQueueFunc([this, m]() {
            std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForAlbum(m.ID);
            for (size_t i = 0; i < v.size(); i++) {
                this->app->sysmodule()->sendAddToSubQueue(v[i].ID);
            }
            this->albumMenu->close();
        });
        this->albumMenu->setAddToPlaylistFunc(nullptr);
        this->albumMenu->setViewInformationFunc([this, m]() {
            // this->changeFrame(Type::AlbumInfo, Action::Push, m.ID);
            // this->albumMenu->close();
        });

        this->albumMenu->resetHighlight();
        this->app->addOverlay(this->albumMenu);
    }

    Artist::~Artist() {
        delete this->albumMenu;
        delete this->menu;
    }
};