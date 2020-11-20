#include "Application.hpp"
#include "lang/Lang.hpp"
#include "Paths.hpp"
#include "ui/element/GridItem.hpp"
#include "ui/element/ScrollableGrid.hpp"
#include "ui/frame/Artist.hpp"
#include "ui/overlay/SortBy.hpp"
#include "utils/Utils.hpp"

// Play button dimensions
#define BUTTON_F 26
#define BUTTON_W 150
#define BUTTON_H 50

// Size of artist image
#define IMAGE_SIZE 160

namespace Frame {
    Artist::Artist(Main::Application * app, ArtistID id) : Frame(app) {
        // Remove list + headings (I should redo Frame to avoid this)
        this->bottomContainer->removeElement(this->list);
        this->topContainer->removeElement(this->titleH);
        this->topContainer->removeElement(this->artistH);
        this->topContainer->removeElement(this->albumH);
        this->topContainer->removeElement(this->lengthH);

        // First get metadata for the provided artist
        this->meta = this->app->database()->getArtistMetadataForID(id);
        if (this->meta.ID < 0) {
            // Helps show there was an error (should never appear)
            this->heading->setString("Artist.Artist"_lang);
            this->albumMenu = nullptr;
            this->artistMenu = nullptr;
            this->sortMenu = nullptr;
            return;
        }

        // Populate with Artist's data
        Aether::Image * image = new Aether::Image(this->x() + 50, this->y() + 50, this->meta.imagePath.empty() ? "romfs:/misc/noartist.png" : this->meta.imagePath);
        image->setWH(IMAGE_SIZE, IMAGE_SIZE);
        this->addElement(image);
        this->heading->setString(this->meta.name);
        this->heading->setX(image->x() + image->w() + 28);
        this->heading->setY(image->y() - 10);
        int maxW = (1280 - this->heading->x() - 30);
        if (this->heading->w() > maxW) {
            Aether::Text * tmp = new Aether::Text(this->heading->x() + maxW, this->heading->y(), "...", 60);
            tmp->setX(tmp->x() - tmp->w());
            this->addElement(tmp);
            this->heading->setW(maxW - tmp->w());
        }

        std::string str;
        if (this->meta.albumCount == 1 && this->meta.songCount == 1) {
            str = "Artist.DetailsOneOne"_lang;

        } else if (this->meta.albumCount == 1) {
            str = Utils::substituteTokens("Artist.DetailsOneMany"_lang, std::to_string(this->meta.songCount));

        } else if (this->meta.songCount == 1) {
            str = Utils::substituteTokens("Artist.DetailsManyOne"_lang, std::to_string(this->meta.albumCount));

        } else {
            str = Utils::substituteTokens("Artist.DetailsManyMany"_lang, std::to_string(this->meta.albumCount), std::to_string(this->meta.songCount));
        }
        this->subHeading->setString(str);
        this->subHeading->setXY(this->heading->x() + 2, this->heading->y() + this->heading->h());

        // Play and 'more' buttons
        this->playButton = new Aether::FilledButton(this->subHeading->x(), this->subHeading->y() + this->subHeading->h() + 20, BUTTON_W, BUTTON_H, "Common.Play"_lang, BUTTON_F, [this]() {
            std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForArtist(this->meta.ID);
            std::vector<SongID> ids;
            for (size_t i = 0; i < v.size(); i++) {
                ids.push_back(v[i].ID);
            }
            this->playNewQueue(this->meta.name, ids, 0, true);
        });
        this->playButton->setFillColour(this->app->theme()->accent());
        this->playButton->setTextColour(Aether::Colour{0, 0, 0, 255});
        this->sort->setY(this->playButton->y());

        Aether::BorderButton * moreButton = new Aether::BorderButton(this->playButton->x() + this->playButton->w() + 20, this->playButton->y(), BUTTON_H, BUTTON_H, 2, "", BUTTON_F, [this, id]() {
            this->createArtistMenu(id);
        });
        moreButton->setBorderColour(this->app->theme()->FG());
        moreButton->setTextColour(this->app->theme()->FG());
        Aether::Image * dots = new Aether::Image(moreButton->x() + moreButton->w()/2, moreButton->y() + moreButton->h()/2, "romfs:/icons/verticaldots.png");
        dots->setXY(dots->x() - dots->w()/2, dots->y() - dots->h()/2);
        dots->setColour(this->app->theme()->FG());
        moreButton->addElement(dots);

        this->topContainer->addElement(this->playButton);
        this->topContainer->addElement(moreButton);

        // Create grid
        int gridY = image->y() + image->h() + 20;
        this->grid = new CustomElm::ScrollableGrid(this->x(), gridY, this->w() - 10, this->h() - gridY, 250, 3);
        this->grid->setShowScrollBar(true);
        this->grid->setScrollBarColour(this->app->theme()->muted2());
        this->bottomContainer->addElement(this->grid);
        this->bottomContainer->setFocussed(this->grid);
        this->createList(Database::SortBy::AlbumAsc);

        // Create sort menu
        this->sort->setCallback([this]() {
            this->app->addOverlay(this->sortMenu);
        });
        std::vector<CustomOvl::SortBy::Entry> sort = {{Database::SortBy::AlbumAsc, "Artist.Sort.AlbumAsc"_lang},
                                                      {Database::SortBy::AlbumDsc, "Artist.Sort.AlbumDsc"_lang},
                                                      {Database::SortBy::SongsAsc, "Artist.Sort.SongsAsc"_lang},
                                                      {Database::SortBy::SongsDsc, "Artist.Sort.SongsDsc"_lang}};
        this->sortMenu = new CustomOvl::SortBy("Album.Sort.Heading"_lang, sort, [this](Database::SortBy s) {
            this->createList(s);
        });
        this->sortMenu->setBackgroundColour(this->app->theme()->popupBG());
        this->sortMenu->setIconColour(this->app->theme()->muted());
        this->sortMenu->setLineColour(this->app->theme()->muted2());
        this->sortMenu->setTextColour(this->app->theme()->FG());

        this->setFocused(this->topContainer);
        this->topContainer->setFocused(this->playButton);

        this->albumMenu = nullptr;
        this->artistMenu = nullptr;
    }

    void Artist::createArtistMenu(ArtistID id) {
        // Create menu if it doesn't exist
        if (this->artistMenu == nullptr) {
            // Query database first
            Metadata::Artist m = this->app->database()->getArtistMetadataForID(id);
            if (m.ID < 0) {
                return;
            }

            this->artistMenu = new CustomOvl::Menu();
            this->artistMenu->setBackgroundColour(this->app->theme()->popupBG());

            // Add to Queue
            CustomElm::MenuButton * b = new CustomElm::MenuButton();
            b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoqueue.png"));
            b->setIconColour(this->app->theme()->muted());
            b->setText("Common.AddToQueue"_lang);
            b->setTextColour(this->app->theme()->FG());
            b->setCallback([this, id]() {
                std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForArtist(id);
                for (size_t i = 0; i < v.size(); i++) {
                    this->app->sysmodule()->sendAddToSubQueue(v[i].ID);
                }
                this->artistMenu->close();
            });
            this->artistMenu->addButton(b);

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
                        this->artistMenu->close();
                    }
                });
            });
            this->artistMenu->addButton(b);
            this->artistMenu->addSeparator(this->app->theme()->muted2());

            // View Information
            b = new CustomElm::MenuButton();
            b->setIcon(new Aether::Image(0, 0, "romfs:/icons/info.png"));
            b->setIconColour(this->app->theme()->muted());
            b->setText("Common.ViewInformation"_lang);
            b->setTextColour(this->app->theme()->FG());
            b->setCallback([this, id]() {
                this->changeFrame(Type::ArtistInfo, Action::Push, id);
                this->artistMenu->close();
            });
            this->artistMenu->addButton(b);

            // Finalize the menu
            this->artistMenu->addButton(nullptr);
        }

        this->artistMenu->resetHighlight();
        this->app->addOverlay(this->artistMenu);
    }

    void Artist::createAlbumMenu(AlbumID id) {
        // Get metadata first
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
        b->setText("Artist.PlayAlbum"_lang);
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
        b->setText("Artist.AddAlbumToQueue"_lang);
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
        b->setText("Artist.AddAlbumToPlaylist"_lang);
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

    void Artist::createList(Database::SortBy sort) {
        // Remove previous items
        this->grid->removeAllElements();

        // Create grid if there are albums
        std::vector<Metadata::Album> md = this->app->database()->getAlbumMetadataForArtist(this->meta.ID, sort);
        if (md.size() > 0) {
            // Populate grid with albums
            for (size_t i = 0; i < md.size(); i++) {
                CustomElm::GridItem * l = new CustomElm::GridItem(md[i].imagePath.empty() ? Path::App::DefaultArtFile : md[i].imagePath);
                l->setMainString(md[i].name);
                std::string str = (md[i].songCount == 1 ? "Common.Song"_lang : Utils::substituteTokens("Common.Songs"_lang, std::to_string(md[i].songCount)));
                l->setSubString(str);
                l->setDotsColour(this->app->theme()->muted());
                l->setTextColour(this->app->theme()->FG());
                l->setMutedTextColour(this->app->theme()->muted());
                AlbumID id = md[i].ID;
                l->setCallback([this, id](){
                    this->changeFrame(Type::Album, Action::Push, id);
                });
                l->setMoreCallback([this, id]() {
                    this->createAlbumMenu(id);
                });
                this->grid->addElement(l);
            }
        }
    }

    void Artist::updateColours() {
        this->playButton->setFillColour(this->app->theme()->accent());
    }

    Artist::~Artist() {
        delete this->albumMenu;
        delete this->artistMenu;
        delete this->sortMenu;
    }
};