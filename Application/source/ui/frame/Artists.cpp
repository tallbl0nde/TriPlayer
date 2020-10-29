#include "Application.hpp"
#include "ui/element/GridItem.hpp"
#include "ui/element/ScrollableGrid.hpp"
#include "ui/frame/Artists.hpp"
#include "ui/overlay/SortBy.hpp"

// Number of GridItems per row
#define COLUMNS 3

namespace Frame {
    Artists::Artists(Main::Application * a) : Frame(a) {
        // Remove list + headings (I should redo Frame to avoid this)
        this->bottomContainer->removeElement(this->list);
        this->topContainer->removeElement(this->titleH);
        this->topContainer->removeElement(this->artistH);
        this->topContainer->removeElement(this->albumH);
        this->topContainer->removeElement(this->lengthH);

        // Now prepare this frame
        this->heading->setString("Artists");
        this->grid = new CustomElm::ScrollableGrid(this->x(), this->y() + 170, this->w() - 10, this->h() - 170, 250, 3);
        this->grid->setShowScrollBar(true);
        this->grid->setScrollBarColour(this->app->theme()->muted2());
        this->bottomContainer->addElement(this->grid);

        // Create sort menu
        this->sort->setCallback([this]() {
            this->app->addOverlay(this->sortMenu);
        });
        std::vector<CustomOvl::SortBy::Entry> sort = {{Database::SortBy::ArtistAsc, "Name (ascending)"},
                                                      {Database::SortBy::ArtistDsc, "Name (descending)"},
                                                      {Database::SortBy::AlbumsAsc, "Album Count (increasing)"},
                                                      {Database::SortBy::AlbumsDsc, "Album Count (decreasing)"},
                                                      {Database::SortBy::SongsAsc, "Song Count (increasing)"},
                                                      {Database::SortBy::SongsDsc, "Song Count (decreasing)"}};
        this->sortMenu = new CustomOvl::SortBy("Sort Artists by", sort, [this](Database::SortBy s) {
            this->createList(s);
        });
        this->sortMenu->setBackgroundColour(this->app->theme()->popupBG());
        this->sortMenu->setIconColour(this->app->theme()->muted());
        this->sortMenu->setLineColour(this->app->theme()->muted2());
        this->sortMenu->setTextColour(this->app->theme()->FG());

        this->createList(Database::SortBy::ArtistAsc);
        this->bottomContainer->setFocussed(this->grid);
        this->menu = nullptr;
    }

    void Artists::createList(Database::SortBy sort) {
        // Remove previous items
        this->grid->removeAllElements();

        // Create items for artists
        std::vector<Metadata::Artist> m = this->app->database()->getAllArtistMetadata(sort);
        if (m.size() > 0) {
            for (size_t i = 0; i < m.size(); i++) {
                std::string img = (m[i].imagePath.empty() ? "romfs:/misc/noartist.png" : m[i].imagePath);
                CustomElm::GridItem * l = new CustomElm::GridItem(img);
                l->setMainString(m[i].name);
                std::string str = std::to_string(m[i].albumCount) + (m[i].albumCount == 1 ? " album" : " albums");
                str += " | " + std::to_string(m[i].songCount) + (m[i].songCount == 1 ? " song" : " songs");
                l->setSubString(str);
                l->setDotsColour(this->app->theme()->muted());
                l->setTextColour(this->app->theme()->FG());
                l->setMutedTextColour(this->app->theme()->muted());
                ArtistID id = m[i].ID;
                l->setCallback([this, id](){
                    this->changeFrame(Type::Artist, Action::Push, id);
                });
                l->setMoreCallback([this, id]() {
                    this->createMenu(id);
                });
                this->grid->addElement(l);
            }
            this->subHeading->setString(std::to_string(m.size()) + (m.size() == 1 ? " artist" : " artists" ));

        // Show message if no artists
        } else {
            this->grid->setHidden(true);
            this->subHeading->setHidden(true);
            Aether::Text * emptyMsg = new Aether::Text(0, grid->y() + grid->h()*0.4, "No artists found!", 24);
            emptyMsg->setColour(this->app->theme()->FG());
            emptyMsg->setX(this->x() + (this->w() - emptyMsg->w())/2);
            this->addElement(emptyMsg);
        }
    }

    void Artists::createMenu(ArtistID id) {
        // Query database first
        Metadata::Artist m = this->app->database()->getArtistMetadataForID(id);
        if (m.ID < 0) {
            return;
        }

        // Create menu
        delete this->menu;
        this->menu = new CustomOvl::ItemMenu();
        this->menu->setBackgroundColour(this->app->theme()->popupBG());
        this->menu->setMainTextColour(this->app->theme()->FG());
        this->menu->setSubTextColour(this->app->theme()->muted());
        this->menu->addSeparator(this->app->theme()->muted2());

        // Set artist specific things
        this->menu->setImage(new Aether::Image(0, 0, m.imagePath.empty() ? "romfs:/misc/noartist.png" : m.imagePath));
        this->menu->setMainText(m.name);
        std::string str = std::to_string(m.albumCount) + (m.albumCount == 1 ? " album" : " albums");
        str += " | " + std::to_string(m.songCount) + (m.songCount == 1 ? " song" : " songs");
        this->menu->setSubText(str);

        // Play All
        CustomElm::MenuButton * b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/playsmall.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Play All");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, m]() {
            std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForArtist(m.ID);
            std::vector<SongID> ids;
            for (size_t i = 0; i < v.size(); i++) {
                ids.push_back(v[i].ID);
            }
            this->playNewQueue(m.name, ids, 0, true);
            this->menu->close();
        });
        this->menu->addButton(b);
        this->menu->addSeparator(this->app->theme()->muted2());

        // Add to Queue
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoqueue.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Add to Queue");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, id]() {
            std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForArtist(id);
            for (size_t i = 0; i < v.size(); i++) {
                this->app->sysmodule()->sendAddToSubQueue(v[i].ID);
            }
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
                    std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForArtist(id);
                    for (size_t j = 0; j < v.size(); j++) {
                        this->app->database()->addSongToPlaylist(i, v[j].ID);
                    }
                    this->menu->close();
                }
            });
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
            this->changeFrame(Type::ArtistInfo, Action::Push, id);
            this->menu->close();
        });
        this->menu->addButton(b);

        // Finalize the menu
        this->menu->addButton(nullptr);
        this->app->addOverlay(this->menu);
    }

    Artists::~Artists() {
        delete this->sortMenu;
        delete this->menu;
    }
};