#include "Application.hpp"
#include "lang/Lang.hpp"
#include "ui/element/GridItem.hpp"
#include "ui/element/ScrollableGrid.hpp"
#include "ui/frame/Artists.hpp"
#include "ui/overlay/SortBy.hpp"
#include "utils/Utils.hpp"

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
        this->heading->setString("Artist.Artists"_lang);
        this->grid = new CustomElm::ScrollableGrid(this->x(), this->y() + 170, this->w() - 10, this->h() - 170, 250, 3);
        this->grid->setShowScrollBar(true);
        this->grid->setScrollBarColour(this->app->theme()->muted2());
        this->bottomContainer->addElement(this->grid);

        // Create sort menu
        this->sort->setCallback([this]() {
            this->app->addOverlay(this->sortMenu);
        });
        std::vector<CustomOvl::SortBy::Entry> sort = {{Database::SortBy::ArtistAsc, "Artist.Sort.ArtistAsc"_lang},
                                                      {Database::SortBy::ArtistDsc, "Artist.Sort.ArtistDsc"_lang},
                                                      {Database::SortBy::AlbumsAsc, "Artist.Sort.AlbumsAsc"_lang},
                                                      {Database::SortBy::AlbumsDsc, "Artist.Sort.AlbumsDsc"_lang},
                                                      {Database::SortBy::SongsAsc, "Artist.Sort.SongsAsc"_lang},
                                                      {Database::SortBy::SongsDsc, "Artist.Sort.SongsDsc"_lang}};
        this->sortMenu = new CustomOvl::SortBy("Artist.Sort.Heading"_lang, sort, [this](Database::SortBy s) {
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
                std::string str;
                if (m[i].albumCount == 1 && m[i].songCount == 1) {
                    str = "Artist.DetailsOneOne"_lang;

                } else if (m[i].albumCount == 1) {
                    str = Utils::substituteTokens("Artist.DetailsOneMany"_lang, std::to_string(m[i].songCount));

                } else if (m[i].songCount == 1) {
                    str = Utils::substituteTokens("Artist.DetailsManyOne"_lang, std::to_string(m[i].albumCount));

                } else {
                    str = Utils::substituteTokens("Artist.DetailsManyMany"_lang, std::to_string(m[i].albumCount), std::to_string(m[i].songCount));
                }
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
            this->subHeading->setString(m.size() == 1 ? "Artist.CountOne"_lang : Utils::substituteTokens("Artist.CountMany"_lang, std::to_string(m.size())));

        // Show message if no artists
        } else {
            this->grid->setHidden(true);
            this->subHeading->setHidden(true);
            Aether::Text * emptyMsg = new Aether::Text(0, grid->y() + grid->h()*0.4, "Artist.NotFound"_lang, 24);
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
        std::string str;
        if (m.albumCount == 1 && m.songCount == 1) {
            str = "Artist.DetailsOneOne"_lang;

        } else if (m.albumCount == 1) {
            str = Utils::substituteTokens("Artist.DetailsOneMany"_lang, std::to_string(m.songCount));

        } else if (m.songCount == 1) {
            str = Utils::substituteTokens("Artist.DetailsManyOne"_lang, std::to_string(m.albumCount));

        } else {
            str = Utils::substituteTokens("Artist.DetailsManyMany"_lang, std::to_string(m.albumCount), std::to_string(m.songCount));
        }
        this->menu->setSubText(str);

        // Play All
        CustomElm::MenuButton * b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/playsmall.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Artist.PlayAll"_lang);
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
        b->setText("Common.AddToQueue"_lang);
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
        b->setText("Common.AddToPlaylist"_lang);
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
        b->setText("Common.ViewInformation"_lang);
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