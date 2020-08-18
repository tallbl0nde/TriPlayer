#include "Application.hpp"
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
        this->subLength->setHidden(true);
        this->subTotal->setHidden(true);
        this->titleH->setHidden(true);
        this->artistH->setHidden(true);
        this->albumH->setHidden(true);
        this->lengthH->setHidden(true);

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
        this->searchThread = std::async(std::launch::async, [this, copy]() -> bool {
           return this->searchDatabase(copy);
        });
    }

    void Search::addEntries() {
        // Set heading and position
        this->heading->setFontSize(46);
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
        this->setFocussed(this->list);
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
                // this->createMenu(id);
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
                // this->createMenu(id);
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
            std::string img = (this->albums[i].imagePath.empty() ? "romfs:/misc/noalbum.png" : this->albums[i].imagePath);
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
                // this->createMenu(id);
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
        this->list->addElement(new Aether::ListSeparator(10));
        if (this->listEmpty) {
            heading->setY(this->list->y() + 10);
            this->listEmpty = false;
        }

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
                this->app->sysmodule()->sendSetPlayingFrom("'" + phrase + "'");
                this->app->sysmodule()->sendSetQueue(this->songIDs);
                this->app->sysmodule()->sendSetSongIdx(i);
                this->app->sysmodule()->sendSetShuffle(this->app->sysmodule()->shuffleMode());
            });
            SongID id = this->songs[i].ID;
            l->setMoreCallback([this, id]() {
                // this->createMenu(id);
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
        this->playlists = this->app->database()->searchPlaylists(phrase);
        this->artists = this->app->database()->searchArtists(phrase);
        this->albums = this->app->database()->searchAlbums(phrase);
        this->songs = this->app->database()->searchSongs(phrase);
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

    void Search::update(uint32_t dt) {
        // Wait until the thread is finished and update frame
        if (!this->threadDone) {
            if (this->searchThread.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                bool result = this->searchThread.get();
                if (!result) {
                    this->showError("An error occurred searching the database. Please restart the application and try again.");
                } else {
                    this->addEntries();
                }
                this->threadDone = true;
            }
        }

        Frame::update(dt);
    }

    Search::~Search() {

    }
};