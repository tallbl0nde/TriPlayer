#include "Application.hpp"
#include "Paths.hpp"
#include "ui/element/GridItem.hpp"
#include "ui/element/HorizontalList.hpp"
#include "ui/element/ListHeadingCount.hpp"
#include "ui/element/listitem/Song.hpp"
#include "ui/frame/Search.hpp"
#include "utils/NX.hpp"
#include "utils/Utils.hpp"

// Keyboard config (see utils/NX.hpp)
static struct Utils::NX::Keyboard keyboard = {
    "",                                                    // buffer
    32,                                                    // max length
    "Search",                                              // ok text
    true,                                                  // show line
    "Search",                                              // heading
    "Search for playlists, artists, albums or songs..."    // subheading
};

namespace Frame {
    Search::Search(Main::Application * a) : Frame(a) {
        // Hide everything
        this->subHeading->setHidden(true);
        this->titleH->setHidden(true);
        this->artistH->setHidden(true);
        this->albumH->setHidden(true);
        this->lengthH->setHidden(true);
        this->sort->setHidden(true);
        this->topContainer->setHasSelectable(false);
        this->artistsList = nullptr;
        this->menu = nullptr;
        this->searchContainer = nullptr;
        this->heading->setString("Results for:");

        // Get input first
        bool haveInput = Utils::NX::getUserInput(keyboard);
        if (!haveInput) {
            // Show error message if we couldn't launch the keyboard
            this->showError("An unexpected error occurred showing the keyboard. If this persists, please restart the application.");
            this->threadDone = true;
            return;
        }

        // Search the database and populate the list!
        std::string copy = keyboard.buffer;
        this->threadDone = false;
        this->showSearching();
        this->searchThread = std::async(std::launch::async, [this, copy]() -> bool {
            Utils::NX::setCPUBoost(true);
            bool ok = this->searchDatabase(copy);
            Utils::NX::setCPUBoost(false);
            return ok;
        });
    }

    void Search::addEntries() {
        // Set heading and position
        this->heading->setString("Results for: " + keyboard.buffer);
        int maxW = (this->w() - (this->heading->x() - this->x())*2);
        if (this->heading->w() > maxW) {
            Aether::Text * tmp = new Aether::Text(this->heading->x() + maxW, this->heading->y(), "...", 46);
            tmp->setX(tmp->x() - tmp->w());
            this->addElement(tmp);
            this->heading->setW(maxW - tmp->w());
        }

        // Position the list
        this->listEmpty = true;
        this->list->setY(this->heading->y() + this->heading->h() + (this->heading->y() - this->y()));
        this->list->setH(this->h() - this->list->y());

        // Show message if no results found
        if (this->playlists.empty() && this->artists.empty() && this->albums.empty() && this->songs.empty()) {
            Aether::Text * text = new Aether::Text(this->x() + this->w()/2, this->y() + this->h()/2, "No results found", 26);
            text->setX(text->x() - text->w()/2);
            text->setColour(this->app->theme()->FG());
            this->addElement(text);
            return;
        }

        // Add elements to list
        this->addPlaylists();
        this->addArtists();
        this->addAlbums();
        this->addSongs();
    }

    void Search::addPlaylists() {
        if (this->playlists.empty()) {
            return;
        }

        // Add heading and count first
        CustomElm::ListHeadingCount * heading = new CustomElm::ListHeadingCount();
        heading->setHeadingString("Playlists");
        heading->setCount(this->playlists.size());
        heading->setTextColour(this->app->theme()->FG());
        this->list->addElement(heading);
        if (this->listEmpty) {
            heading->setY(this->list->y() + 10);
            this->listEmpty = false;
        }

        // Create horizontal list and populate with items
        CustomElm::HorizontalList * hlist = new CustomElm::HorizontalList(this->list->x(), 0, this->list->w(), 250);
        for (size_t i = 0; i < this->playlists.size(); i++) {
            std::string img = (this->playlists[i].imagePath.empty() ? "romfs:/misc/noplaylist.png" : this->playlists[i].imagePath);
            CustomElm::GridItem * l = new CustomElm::GridItem(img);
            l->setMainString(this->playlists[i].name);
            l->setSubString(std::to_string(this->playlists[i].songCount) + (this->playlists[i].songCount == 1 ? " song" : " songs"));
            l->setDotsColour(this->app->theme()->muted());
            l->setTextColour(this->app->theme()->FG());
            l->setMutedTextColour(this->app->theme()->muted());
            PlaylistID id = this->playlists[i].ID;
            l->setCallback([this, id](){
                this->changeFrame(Type::Playlist, Action::Push, id);
            });
            l->setMoreCallback([this, id]() {
                this->createPlaylistMenu(id);
            });
            hlist->addElement(l);
        }
        this->list->addElement(hlist);
        this->playlists.clear();
    }

    void Search::addArtists() {
        if (this->artists.empty()) {
            return;
        }

        // Add heading and count first
        CustomElm::ListHeadingCount * heading = new CustomElm::ListHeadingCount();
        heading->setHeadingString("Artists");
        heading->setCount(this->artists.size());
        heading->setTextColour(this->app->theme()->FG());
        this->list->addElement(heading);
        if (this->listEmpty) {
            heading->setY(this->list->y() + 10);
            this->listEmpty = false;
        }

        // Create horizontal list and populate with items
        CustomElm::HorizontalList * hlist = new CustomElm::HorizontalList(this->list->x(), 0, this->list->w(), 250);
        for (size_t i = 0; i < this->artists.size(); i++) {
            std::string img = (this->artists[i].imagePath.empty() ? "romfs:/misc/noartist.png" : this->artists[i].imagePath);
            CustomElm::GridItem * l = new CustomElm::GridItem(img);
            l->setMainString(this->artists[i].name);
            std::string str = std::to_string(this->artists[i].albumCount) + (this->artists[i].albumCount == 1 ? " album" : " albums");
            str += " | " + std::to_string(this->artists[i].songCount) + (this->artists[i].songCount == 1 ? " song" : " songs");
            l->setSubString(str);
            l->setDotsColour(this->app->theme()->muted());
            l->setTextColour(this->app->theme()->FG());
            l->setMutedTextColour(this->app->theme()->muted());
            ArtistID id = this->artists[i].ID;
            l->setCallback([this, id](){
                this->changeFrame(Type::Artist, Action::Push, id);
            });
            l->setMoreCallback([this, id]() {
                this->createArtistMenu(id);
            });
            hlist->addElement(l);
        }
        this->list->addElement(hlist);
        this->artists.clear();
    }

    void Search::addAlbums() {
        if (this->albums.empty()) {
            return;
        }

        // Add heading and count first
        CustomElm::ListHeadingCount * heading = new CustomElm::ListHeadingCount();
        heading->setHeadingString("Albums");
        heading->setCount(this->albums.size());
        heading->setTextColour(this->app->theme()->FG());
        this->list->addElement(heading);
        if (this->listEmpty) {
            heading->setY(this->list->y() + 10);
            this->listEmpty = false;
        }

        // Create horizontal list and populate with items
        CustomElm::HorizontalList * hlist = new CustomElm::HorizontalList(this->list->x(), 0, this->list->w(), 250);
        for (size_t i = 0; i < this->albums.size(); i++) {
            std::string img = (this->albums[i].imagePath.empty() ? Path::App::DefaultArtFile : this->albums[i].imagePath);
            CustomElm::GridItem * l = new CustomElm::GridItem(img);
            l->setMainString(this->albums[i].name);
            l->setSubString(this->albums[i].artist);
            l->setDotsColour(this->app->theme()->muted());
            l->setTextColour(this->app->theme()->FG());
            l->setMutedTextColour(this->app->theme()->muted());
            AlbumID id = this->albums[i].ID;
            l->setCallback([this, id](){
                this->changeFrame(Type::Album, Action::Push, id);
            });
            l->setMoreCallback([this, id]() {
                this->createAlbumMenu(id);
            });
            hlist->addElement(l);
        }
        this->list->addElement(hlist);
        this->albums.clear();
    }

    void Search::addSongs() {
        if (this->songs.empty()) {
            return;
        }

        // Add heading and count first
        CustomElm::ListHeadingCount * heading = new CustomElm::ListHeadingCount();
        heading->setHeadingString("Songs");
        heading->setCount(this->songs.size());
        heading->setTextColour(this->app->theme()->FG());
        this->list->addElement(heading);
        if (this->listEmpty) {
            heading->setY(this->list->y() + 10);
            this->listEmpty = false;
        }
        this->list->addElement(new Aether::ListSeparator(10));

        // Add songs to list
        for (size_t i = 0; i < this->songs.size(); i++) {
            this->songIDs.push_back(this->songs[i].ID);
            CustomElm::ListItem::Song * l = new CustomElm::ListItem::Song();
            l->setTitleString(this->songs[i].title);
            l->setArtistString(this->songs[i].artist);
            l->setAlbumString(this->songs[i].album);
            l->setLengthString(Utils::secondsToHMS(this->songs[i].duration));
            l->setLineColour(this->app->theme()->muted2());
            l->setMoreColour(this->app->theme()->muted());
            l->setTextColour(this->app->theme()->FG());
            std::string phrase = keyboard.buffer;
            l->setCallback([this, phrase, i](){
                this->playNewQueue("'" + phrase + "'", this->songIDs, i, false);
            });
            SongID id = this->songs[i].ID;
            l->setMoreCallback([this, id]() {
                this->createSongMenu(id);
            });
            this->list->addElement(l);
        }
        this->songs.clear();
    }

    bool Search::searchDatabase(const std::string & phrase) {
        // Ensure the database is up to date
        if (this->app->database()->needsSearchUpdate()) {
            this->app->lockDatabase();
            bool ok = this->app->database()->prepareSearch();
            this->app->unlockDatabase();

            if (!ok) {
                return false;
            }
        }

        // Search for each type of entry and return
        this->playlists = this->app->database()->searchPlaylists(phrase, this->app->config()->searchMaxPlaylists());
        this->artists = this->app->database()->searchArtists(phrase, this->app->config()->searchMaxArtists());
        this->albums = this->app->database()->searchAlbums(phrase, this->app->config()->searchMaxAlbums());
        this->songs = this->app->database()->searchSongs(phrase, this->app->config()->searchMaxSongs());
        return true;
    }

    void Search::showError(const std::string & message) {
        this->heading->setHidden(true);

        // ! icon
        Aether::Image * icon = new Aether::Image(0, 0, "romfs:/icons/exclamation.png");
        icon->setWH(100, 100);
        icon->setXY(this->x() + (this->w() - icon->w())/2, this->y() + this->h()/4);
        icon->setColour(this->app->theme()->FG());
        this->addElement(icon);

        // Message
        Aether::TextBlock * msg = new Aether::TextBlock(0, icon->y() + icon->h() + 30, message, 26, this->w()*0.75);
        msg->setX(this->x() + (this->w() - msg->w())/2);
        msg->setColour(this->app->theme()->FG());
        this->addElement(msg);
    }

    void Search::showSearching() {
        this->searchContainer = new Aether::Container(0, 0, 500, 60);
        this->searchContainer->setXY(this->x() + (this->w() - this->searchContainer->w())/2, this->y() + this->h()/2);

        // Text
        Aether::Text * text = new Aether::Text(0, 0, "Searching...", 26);
        text->setXY(this->searchContainer->x() + (this->searchContainer->w() - text->w())/2 + 20, this->searchContainer->y());
        text->setColour(this->app->theme()->FG());
        this->searchContainer->addElement(text);

        // Animation
        Aether::Animation * anim = new Aether::Animation(text->x() - 50, text->y() + (text->h() - 20)/2, 40, 20);
        for (size_t i = 1; i <= 50; i++) {
            Aether::Image * im = new Aether::Image(anim->x(), anim->y(), "romfs:/anim/infload/" + std::to_string(i) + ".png");
            im->setWH(40, 20);
            im->setColour(this->app->theme()->accent());
            anim->addElement(im);
        }
        anim->setAnimateSpeed(50);
        this->searchContainer->addElement(anim);

        this->addElement(this->searchContainer);
    }

    void Search::createNewMenu() {
        // Delete old menu and recreate
        delete this->menu;
        this->menu = new CustomOvl::ItemMenu();
        this->menu->setBackgroundColour(this->app->theme()->popupBG());
        this->menu->setMainTextColour(this->app->theme()->FG());
        this->menu->setSubTextColour(this->app->theme()->muted());
        this->menu->addSeparator(this->app->theme()->muted2());
    }

    void Search::createPlaylistMenu(PlaylistID id) {
        this->createNewMenu();

        // Set playlist specific things
        Metadata::Playlist m = this->app->database()->getPlaylistMetadataForID(id);
        this->menu->setImage(new Aether::Image(0, 0, m.imagePath.empty() ? "romfs:/misc/noplaylist.png" : m.imagePath));
        this->menu->setMainText(m.name);
        std::string str = std::to_string(m.songCount) + (m.songCount == 1 ? " song" : " songs");
        this->menu->setSubText(str);

        // Play
        CustomElm::MenuButton * b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/playsmall.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Play");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, m]() {
            std::vector<Metadata::PlaylistSong> v = this->app->database()->getSongMetadataForPlaylist(m.ID, Database::SortBy::TitleAsc);
            if (v.size() > 0) {
                std::vector<SongID> ids;
                for (size_t i = 0; i < v.size(); i++) {
                    ids.push_back(v[i].song.ID);
                }
                this->playNewQueue(m.name, ids, 0, true);
            }
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
        b->setCallback([this, m]() {
            std::vector<Metadata::PlaylistSong> v = this->app->database()->getSongMetadataForPlaylist(m.ID, Database::SortBy::TitleAsc);
            for (size_t i = 0; i < v.size(); i++) {
                this->app->sysmodule()->sendAddToSubQueue(v[i].song.ID);
            }
            this->menu->close();
        });
        this->menu->addButton(b);

        // Add songs to another Playlist
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoplaylist.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Add to other Playlist");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, m]() {
            this->showAddToPlaylist([this, m](PlaylistID i) {
                if (i >= 0) {
                    // Get list of songs and add one-by-one to other playlist
                    std::vector<Metadata::PlaylistSong> v = this->app->database()->getSongMetadataForPlaylist(m.ID, Database::SortBy::TitleAsc);
                    for (size_t j = 0; j < v.size(); j++) {
                        this->app->database()->addSongToPlaylist(i, v[j].song.ID);
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
        b->setCallback([this, m]() {
            this->changeFrame(Type::PlaylistInfo, Action::Push, m.ID);
            this->menu->close();
        });
        this->menu->addButton(b);

        this->menu->addButton(nullptr);
        this->app->addOverlay(this->menu);
    }

    void Search::createArtistMenu(ArtistID id) {
        this->createNewMenu();

        // Set artist specific things
        Metadata::Artist m = this->app->database()->getArtistMetadataForID(id);
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

        this->menu->addButton(nullptr);
        this->app->addOverlay(this->menu);
    }

    void Search::createAlbumMenu(AlbumID id) {
        this->createNewMenu();

        // Album metadata
        Metadata::Album m = this->app->database()->getAlbumMetadataForID(id);
        this->menu->setImage(new Aether::Image(0, 0, m.imagePath.empty() ? Path::App::DefaultArtFile : m.imagePath));
        this->menu->setMainText(m.name);
        this->menu->setSubText(m.artist);

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
            std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForAlbum(id);
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
                    std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForAlbum(id);
                    for (size_t j = 0; j < v.size(); j++) {
                        this->app->database()->addSongToPlaylist(i, v[j].ID);
                    }
                    this->menu->close();
                }
            });
        });
        this->menu->addButton(b);
        this->menu->addSeparator(this->app->theme()->muted2());

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
                this->menu->close();
            });

        } else {
            b->setText("View Artists");
            b->setCallback([this, id]() {
                this->createArtistsList(id);
                this->menu->close();
            });
        }
        this->menu->addButton(b);
        this->menu->addSeparator(this->app->theme()->muted2());

        // View Information
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/info.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("View Information");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, id]() {
            this->changeFrame(Type::AlbumInfo, Action::Push, id);
            this->menu->close();
        });
        this->menu->addButton(b);

        this->menu->addButton(nullptr);
        this->app->addOverlay(this->menu);
    }

    void Search::createSongMenu(SongID id) {
        this->createNewMenu();

        // Song metadata
        Metadata::Song m = this->app->database()->getSongMetadataForID(id);
        this->menu->setMainText(m.title);
        this->menu->setSubText(m.artist);
        AlbumID aID = this->app->database()->getAlbumIDForSong(m.ID);
        Metadata::Album md = this->app->database()->getAlbumMetadataForID(aID);
        this->menu->setImage(new Aether::Image(0, 0, md.imagePath.empty() ? Path::App::DefaultArtFile : md.imagePath));

        // Add to Queue
        CustomElm::MenuButton * b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoqueue.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Add to Queue");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, id]() {
            this->app->sysmodule()->sendAddToSubQueue(id);
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
                    this->app->database()->addSongToPlaylist(i, id);
                    this->menu->close();
                }
            });
        });
        this->menu->addButton(b);
        this->menu->addSeparator(this->app->theme()->muted2());

        // Go to Artist
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/user.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Go to Artist");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, id]() {
            ArtistID a = this->app->database()->getArtistIDForSong(id);
            if (a >= 0) {
                this->changeFrame(Type::Artist, Action::Push, a);
            }
            this->menu->close();
        });
        this->menu->addButton(b);

        // Go to Album
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/disc.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Go to Album");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, id]() {
            AlbumID a = this->app->database()->getAlbumIDForSong(id);
            if (a >= 0) {
                this->changeFrame(Type::Album, Action::Push, a);
            }
            this->menu->close();
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
            this->changeFrame(Type::SongInfo, Action::Push, id);
            this->menu->close();
        });
        this->menu->addButton(b);

        this->menu->addButton(nullptr);
        this->app->addOverlay(this->menu);
    }

    void Search::createArtistsList(AlbumID id) {
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

    void Search::update(uint32_t dt) {
        // Wait until the thread is finished and update frame
        if (!this->threadDone) {
            if (this->searchThread.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                bool result = this->searchThread.get();
                if (!result) {
                    this->showError("An error occurred searching the database. Please restart the application and try again.");
                } else {
                    this->removeElement(this->searchContainer);
                    this->addEntries();
                }
                this->threadDone = true;
            }
        }

        Frame::update(dt);
    }

    Search::~Search() {
        delete this->artistsList;
        delete this->menu;
    }
};