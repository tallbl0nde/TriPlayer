#include "Application.hpp"
#include "ui/element/GridItem.hpp"
#include "ui/element/ScrollableGrid.hpp"
#include "ui/frame/Artist.hpp"

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
            this->menu->resetHighlight();
            this->app->addOverlay(this->menu);
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
                CustomElm::GridItem * l = new CustomElm::GridItem("romfs:/misc/noalbum.png");
                l->setMainString(md[i].name);
                l->setSubString("? songs");
                l->setDotsColour(this->app->theme()->muted());
                l->setTextColour(this->app->theme()->FG());
                l->setMutedTextColour(this->app->theme()->muted());
                AlbumID id = md[i].ID;
                l->setCallback([this, id](){
                    // this->changeFrame(Type::Album, Action::Push, id);
                });
                l->setMoreCallback([this, id]() {
                    // this->createAlbumMenu(id);
                });
                grid->addElement(l);
            }

            this->addElement(grid);
        }

        // Create menu which is shown when the dots are pressed
        this->menu = new CustomOvl::Menu::Artist(CustomOvl::Menu::Type::HideTop);
        this->menu->setPlayAllText("Play All");
        this->menu->setAddToQueueText("Add to Queue");
        this->menu->setAddToPlaylistText("Add to Playlist");
        this->menu->setViewInformationText("View Information");
        this->menu->setBackgroundColour(this->app->theme()->popupBG());
        this->menu->setIconColour(this->app->theme()->muted());
        this->menu->setLineColour(this->app->theme()->muted2());
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

    Artist::~Artist() {
        delete this->menu;
    }
};