#include "Application.hpp"
#include "Paths.hpp"
#include "ui/element/GridItem.hpp"
#include "ui/element/listitem/Artist.hpp"
#include "ui/element/ScrollableGrid.hpp"
#include "ui/frame/Albums.hpp"
#include "ui/overlay/SortBy.hpp"

// Number of GridItems per row
#define COLUMNS 3

namespace Frame {
    Albums::Albums(Main::Application * a) : Frame(a) {
        // Remove list + headings (I should redo Frame to avoid this)
        this->bottomContainer->removeElement(this->list);
        this->topContainer->removeElement(this->titleH);
        this->topContainer->removeElement(this->artistH);
        this->topContainer->removeElement(this->albumH);
        this->topContainer->removeElement(this->lengthH);

        // Now prepare this frame
        this->heading->setString("Album.Albums"_lang);
        this->grid = new CustomElm::ScrollableGrid(this->x(), this->y() + 170, this->w() - 10, this->h() - 170, 250, 3);
        this->grid->setShowScrollBar(true);
        this->grid->setScrollBarColour(this->app->theme()->muted2());
        this->bottomContainer->addElement(this->grid);

        // Create sort menu
        this->sort->setCallback([this]() {
            this->app->addOverlay(this->sortMenu);
        });
        std::vector<CustomOvl::SortBy::Entry> sort = {{Database::SortBy::AlbumAsc, "Album.Sort.AlbumAsc"_lang},
                                                      {Database::SortBy::AlbumDsc, "Album.Sort.AlbumDsc"_lang},
                                                      {Database::SortBy::ArtistAsc, "Album.Sort.ArtistAsc"_lang},
                                                      {Database::SortBy::ArtistDsc, "Album.Sort.ArtistDsc"_lang}};
        this->sortMenu = new CustomOvl::SortBy("Album.Sort.Heading"_lang, sort, [this](Database::SortBy s) {
            this->createList(s);
        });
        this->sortMenu->setBackgroundColour(this->app->theme()->popupBG());
        this->sortMenu->setIconColour(this->app->theme()->muted());
        this->sortMenu->setLineColour(this->app->theme()->muted2());
        this->sortMenu->setTextColour(this->app->theme()->FG());

        this->createList(Database::SortBy::AlbumAsc);
        this->bottomContainer->setFocussed(this->grid);
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

    void Albums::createList(Database::SortBy sort) {
        // Remove previous items
        this->grid->removeAllElements();

        // Create items for albums
        std::vector<Metadata::Album> m = this->app->database()->getAllAlbumMetadata(sort);
        if (m.size() > 0) {
            for (size_t i = 0; i < m.size(); i++) {
                std::string img = (m[i].imagePath.empty() ? Path::App::DefaultArtFile : m[i].imagePath);
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
                this->grid->addElement(l);
            }
            this->subHeading->setString((m.size() == 1 ? "Album.CountOne"_lang : Utils::regexReplace("Album.CountMany"_lang, std::to_string(m.size()))));

        // Show message if no albums
        } else {
            this->grid->setHidden(true);
            this->subHeading->setHidden(true);
            Aether::Text * emptyMsg = new Aether::Text(0, grid->y() + grid->h()*0.4, "Album.NotFound"_lang, 24);
            emptyMsg->setColour(this->app->theme()->FG());
            emptyMsg->setX(this->x() + (this->w() - emptyMsg->w())/2);
            this->addElement(emptyMsg);
        }

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
        this->albumMenu->setImage(new Aether::Image(0, 0, m.imagePath.empty() ? Path::App::DefaultArtFile : m.imagePath));
        this->albumMenu->setMainText(m.name);
        this->albumMenu->setSubText(m.artist);

        // Play Album
        CustomElm::MenuButton * b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/playsmall.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Common.Play"_lang);
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, m]() {
            std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForAlbum(m.ID);
            std::vector<SongID> ids;
            for (size_t i = 0; i < v.size(); i++) {
                ids.push_back(v[i].ID);
            }
            this->playNewQueue(m.name, ids, 0, true);
            this->albumMenu->close();
        });
        this->albumMenu->addButton(b);
        this->albumMenu->addSeparator(this->app->theme()->muted2());

        // Add to Queue
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoqueue.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Common.AddToQueue"_lang);
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
        b->setText("Common.AddToPlaylist"_lang);
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, id]() {
            this->showAddToPlaylist([this, id](PlaylistID i) {
                if (i >= 0) {
                    std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForAlbum(id);
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
        if (m.artist != "Various Artists") {
            b->setText("Common.GoToArtist"_lang);
            ArtistID aID = this->app->database()->getArtistIDForName(m.artist);
            b->setCallback([this, aID]() {
                this->changeFrame(Type::Artist, Action::Push, aID);
                this->albumMenu->close();
            });

        } else {
            b->setText("Common.ViewArtists"_lang);
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
        b->setText("Common.ViewInformation"_lang);
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, id]() {
            this->changeFrame(Type::AlbumInfo, Action::Push, id);
            this->albumMenu->close();
        });
        this->albumMenu->addButton(b);

        // Finalize the menu
        this->albumMenu->addButton(nullptr);
        this->app->addOverlay(this->albumMenu);
    }

    Albums::~Albums() {
        delete this->artistsList;
        delete this->albumMenu;
        delete this->sortMenu;
    }
};