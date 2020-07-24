#include "Application.hpp"
#include "ui/element/GridItem.hpp"
#include "ui/element/ListArtist.hpp"
#include "ui/element/ScrollableGrid.hpp"
#include "ui/frame/Albums.hpp"

// Number of GridItems per row
#define COLUMNS 3

namespace Frame {
    Albums::Albums(Main::Application * a) : Frame(a) {
        // Remove list + headings (I should redo Frame to avoid this)
        this->removeElement(this->list);
        this->removeElement(this->titleH);
        this->removeElement(this->artistH);
        this->removeElement(this->albumH);
        this->removeElement(this->lengthH);

        // Now prepare this frame
        this->heading->setString("Albums");
        CustomElm::ScrollableGrid * grid = new CustomElm::ScrollableGrid(this->x(), this->y() + 150, this->w() - 10, this->h() - 150, 250, 3);
        grid->setShowScrollBar(true);
        grid->setScrollBarColour(this->app->theme()->muted2());

        // Create items for albums
        std::vector<Metadata::Album> m = this->app->database()->getAllAlbumMetadata();
        if (m.size() > 0) {
            for (size_t i = 0; i < m.size(); i++) {
                std::string img = (m[i].imagePath.empty() ? "romfs:/misc/noalbum.png" : m[i].imagePath);
                CustomElm::GridItem * l = new CustomElm::GridItem(img);
                l->setMainString(m[i].name);
                l->setSubString(m[i].artist);
                l->setDotsColour(this->app->theme()->muted());
                l->setTextColour(this->app->theme()->FG());
                l->setMutedTextColour(this->app->theme()->muted());
                AlbumID id = m[i].ID;
                l->setCallback([this, id](){
                    this->changeFrame(Type::Album, Action::Push, id);
                });
                l->setMoreCallback([this, id]() {
                    this->createMenu(id);
                });
                grid->addElement(l);
            }

            this->subLength->setHidden(true);
            this->subTotal->setString(std::to_string(m.size()) + (m.size() == 1 ? " album" : " albums" ));
            this->subTotal->setX(this->x() + 885 - this->subTotal->w());

            this->addElement(grid);
            this->setFocussed(grid);

        // Show message if no albums
        } else {
            grid->setHidden(true);
            this->subLength->setHidden(true);
            this->subTotal->setHidden(true);
            Aether::Text * emptyMsg = new Aether::Text(0, grid->y() + grid->h()*0.4, "No albums found!", 24);
            emptyMsg->setColour(this->app->theme()->FG());
            emptyMsg->setX(this->x() + (this->w() - emptyMsg->w())/2);
            this->addElement(emptyMsg);
        }

        this->artistsList = nullptr;
        this->albumMenu = nullptr;
    }

    void Albums::createArtistsList(AlbumID id) {
        // Query database for artists first
        std::vector<Metadata::Artist> m = this->app->database()->getArtistMetadataForAlbum(id);

        // Create menu
        delete this->artistsList;
        this->artistsList = new CustomOvl::ArtistList();
        this->artistsList->setBackgroundColour(this->app->theme()->popupBG());

        // Populate with artists
        for (size_t i = 0; i < m.size(); i++) {
            CustomElm::ListArtist * l = new CustomElm::ListArtist(m[i].imagePath.empty() ? "romfs:/misc/noartist.png" : m[i].imagePath);
            l->setName(m[i].name);
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

    void Albums::createMenu(AlbumID id) {
        // Query database first
        Metadata::Album m = this->app->database()->getAlbumMetadataForID(id);
        if (m.ID < 0) {
            return;
        }

        // Create menu
        delete this->albumMenu;
        this->albumMenu = new CustomOvl::ItemMenu();
        this->albumMenu->setBackgroundColour(this->app->theme()->popupBG());
        this->albumMenu->setMainTextColour(this->app->theme()->FG());
        this->albumMenu->setSubTextColour(this->app->theme()->muted());
        this->albumMenu->addSeparator(this->app->theme()->muted2());

        // Album metadata
        this->albumMenu->setImage(new Aether::Image(0, 0, m.imagePath.empty() ? "romfs:/misc/noalbum.png" : m.imagePath));
        this->albumMenu->setMainText(m.name);
        this->albumMenu->setSubText(m.artist);

        // Play Album
        CustomElm::MenuButton * b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/playsmall.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Play");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, m]() {
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
        this->albumMenu->addButton(b);
        this->albumMenu->addSeparator(this->app->theme()->muted2());

        // Add to Queue
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoqueue.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Add to Queue");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, id]() {
            std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForAlbum(id);
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
        b->setCallback([this, id]() {
            // Do something
        });
        this->albumMenu->addButton(b);
        this->albumMenu->addSeparator(this->app->theme()->muted2());

        // View Artist(s)
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/user.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setTextColour(this->app->theme()->FG());
        if (m.artist != "Various Artists") {
            b->setText("Go to Artist");
            ArtistID aID = this->app->database()->getArtistIDForName(m.artist);
            b->setCallback([this, aID]() {
                this->changeFrame(Type::Artist, Action::Push, aID);
                this->albumMenu->close();
            });

        } else {
            b->setText("View Artists");
            b->setCallback([this, id]() {
                this->createArtistsList(id);
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
        b->setCallback([this, id]() {
            // this->changeFrame(Type::AlbumInfo, Action::Push, id);
        });
        this->albumMenu->addButton(b);

        // Finalize the menu
        this->albumMenu->addButton(nullptr);
        this->app->addOverlay(this->albumMenu);
    }

    Albums::~Albums() {
        delete this->artistsList;
        delete this->albumMenu;
    }
};