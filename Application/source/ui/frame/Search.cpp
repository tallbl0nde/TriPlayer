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
        this->prepareBox = nullptr;

        // Get input first
        bool haveInput = Utils::NX::getUserInput(keyboard);
        if (!haveInput) {
            // Show error message if we couldn't launch the keyboard
            this->showError("An unexpected error occurred showing the keyboard. If this persists, please restart the application.");
            return;
        }

        // Ensure the database is up to date
        if (this->app->database()->needsSearchUpdate()) {
            this->createPreparingOverlay();
            this->app->lockDatabase();
            bool ok = this->app->database()->prepareSearch();
            this->app->unlockDatabase();
            this->prepareBox->close();

            // Show error message if needed
            if (!ok) {
                this->showError("An error occurred updating the search data. Please restart the application and try again.");
                return;
            }
        }

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
        this->list->setY(this->heading->y() + this->heading->h() + (this->heading->y() - this->y()));
        this->list->setH(this->h() - this->list->y());

        // Search the database and populate the list!
        this->listEmpty = true;
        this->searchDatabase(keyboard.buffer);
    }

    void Search::addPlaylists(const std::vector<Metadata::Playlist> & v) {
        // Add heading and count first
        CustomElm::ListHeadingCount * heading = new CustomElm::ListHeadingCount();
        heading->setHeadingString("Playlists");
        heading->setCount(v.size());
        heading->setTextColour(this->app->theme()->FG());
        this->list->addElement(heading);
        if (this->listEmpty) {
            heading->setY(this->list->y() + 10);
            this->listEmpty = false;
        }

        // Create horizontal list and populate with items
        CustomElm::HorizontalList * hlist = new CustomElm::HorizontalList(this->list->x(), 0, this->list->w(), 250);
        for (size_t i = 0; i < v.size(); i++) {
            std::string img = (v[i].imagePath.empty() ? "romfs:/misc/noplaylist.png" : v[i].imagePath);
            CustomElm::GridItem * l = new CustomElm::GridItem(img);
            l->setMainString(v[i].name);
            l->setSubString(std::to_string(v[i].songCount) + (v[i].songCount == 1 ? " song" : " songs"));
            l->setDotsColour(this->app->theme()->muted());
            l->setTextColour(this->app->theme()->FG());
            l->setMutedTextColour(this->app->theme()->muted());
            PlaylistID id = v[i].ID;
            l->setCallback([this, id](){
                this->changeFrame(Type::Playlist, Action::Push, id);
            });
            l->setMoreCallback([this, id]() {
                // this->createMenu(id);
            });
            hlist->addElement(l);
        }
        this->list->addElement(hlist);
    }

    void Search::addArtists(const std::vector<Metadata::Artist> & v) {
        // Add heading and count first
        CustomElm::ListHeadingCount * heading = new CustomElm::ListHeadingCount();
        heading->setHeadingString("Artists");
        heading->setCount(v.size());
        heading->setTextColour(this->app->theme()->FG());
        this->list->addElement(heading);
        if (this->listEmpty) {
            heading->setY(this->list->y() + 10);
            this->listEmpty = false;
        }

        // Create horizontal list and populate with items
        CustomElm::HorizontalList * hlist = new CustomElm::HorizontalList(this->list->x(), 0, this->list->w(), 250);
        for (size_t i = 0; i < v.size(); i++) {
            std::string img = (v[i].imagePath.empty() ? "romfs:/misc/noartist.png" : v[i].imagePath);
            CustomElm::GridItem * l = new CustomElm::GridItem(img);
            l->setMainString(v[i].name);
            std::string str = std::to_string(v[i].albumCount) + (v[i].albumCount == 1 ? " album" : " albums");
            str += " | " + std::to_string(v[i].songCount) + (v[i].songCount == 1 ? " song" : " songs");
            l->setSubString(str);
            l->setDotsColour(this->app->theme()->muted());
            l->setTextColour(this->app->theme()->FG());
            l->setMutedTextColour(this->app->theme()->muted());
            ArtistID id = v[i].ID;
            l->setCallback([this, id](){
                this->changeFrame(Type::Artist, Action::Push, id);
            });
            l->setMoreCallback([this, id]() {
                // this->createMenu(id);
            });
            hlist->addElement(l);
        }
        this->list->addElement(hlist);
    }

    void Search::addAlbums(const std::vector<Metadata::Album> & v) {
        // Add heading and count first
        CustomElm::ListHeadingCount * heading = new CustomElm::ListHeadingCount();
        heading->setHeadingString("Albums");
        heading->setCount(v.size());
        heading->setTextColour(this->app->theme()->FG());
        this->list->addElement(heading);
        if (this->listEmpty) {
            heading->setY(this->list->y() + 10);
            this->listEmpty = false;
        }

        // Create horizontal list and populate with items
        CustomElm::HorizontalList * hlist = new CustomElm::HorizontalList(this->list->x(), 0, this->list->w(), 250);
        for (size_t i = 0; i < v.size(); i++) {
            std::string img = (v[i].imagePath.empty() ? "romfs:/misc/noalbum.png" : v[i].imagePath);
            CustomElm::GridItem * l = new CustomElm::GridItem(img);
            l->setMainString(v[i].name);
            l->setSubString(v[i].artist);
            l->setDotsColour(this->app->theme()->muted());
            l->setTextColour(this->app->theme()->FG());
            l->setMutedTextColour(this->app->theme()->muted());
            AlbumID id = v[i].ID;
            l->setCallback([this, id](){
                this->changeFrame(Type::Album, Action::Push, id);
            });
            l->setMoreCallback([this, id]() {
                // this->createMenu(id);
            });
            hlist->addElement(l);
        }
        this->list->addElement(hlist);
    }

    void Search::addSongs(const std::vector<Metadata::Song> & v, const std::string & phrase) {
        // Add heading and count first
        CustomElm::ListHeadingCount * heading = new CustomElm::ListHeadingCount();
        heading->setHeadingString("Songs");
        heading->setCount(v.size());
        heading->setTextColour(this->app->theme()->FG());
        this->list->addElement(heading);
        this->list->addElement(new Aether::ListSeparator(10));
        if (this->listEmpty) {
            heading->setY(this->list->y() + 10);
            this->listEmpty = false;
        }

        // Add songs to list
        for (size_t i = 0; i < v.size(); i++) {
            this->songIDs.push_back(v[i].ID);
            CustomElm::ListItem::Song * l = new CustomElm::ListItem::Song();
            l->setTitleString(v[i].title);
            l->setArtistString(v[i].artist);
            l->setAlbumString(v[i].album);
            l->setLengthString(Utils::secondsToHMS(v[i].duration));
            l->setLineColour(this->app->theme()->muted2());
            l->setMoreColour(this->app->theme()->muted());
            l->setTextColour(this->app->theme()->FG());
            l->setCallback([this, phrase, i](){
                this->app->sysmodule()->sendSetPlayingFrom("'" + phrase + "'");
                this->app->sysmodule()->sendSetQueue(this->songIDs);
                this->app->sysmodule()->sendSetSongIdx(i);
                this->app->sysmodule()->sendSetShuffle(this->app->sysmodule()->shuffleMode());
            });
            SongID id = v[i].ID;
            l->setMoreCallback([this, id]() {
                // this->createMenu(id);
            });
            this->list->addElement(l);
        }
    }

    void Search::searchDatabase(const std::string & phrase) {
        // Search for each entry first
        std::vector<Metadata::Playlist> playlists = this->app->database()->searchPlaylists(phrase);
        std::vector<Metadata::Artist> artists = this->app->database()->searchArtists(phrase);
        std::vector<Metadata::Album> albums = this->app->database()->searchAlbums(phrase);
        std::vector<Metadata::Song> songs = this->app->database()->searchSongs(phrase);

        // Show message if no results found
        if (playlists.empty() && artists.empty() && albums.empty() && songs.empty()) {
            Aether::Text * text = new Aether::Text(this->x() + this->w()/2, this->y() + this->h()/2, "No results found", 26);
            text->setX(text->x() - text->w()/2);
            text->setColour(this->app->theme()->FG());
            this->addElement(text);
            return;
        }

        // Add elements if needed
        if (!playlists.empty()) {
            this->addPlaylists(playlists);
        }
        if (!artists.empty()) {
            this->addArtists(artists);
        }
        if (!albums.empty()) {
            this->addAlbums(albums);
        }
        if (!songs.empty()) {
            this->addSongs(songs, phrase);
        }
        this->setFocussed(this->list);
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

    void Search::createPreparingOverlay() {
        this->prepareBox = new Aether::MessageBox();
        this->prepareBox->setLineColour(this->app->theme()->muted2());
        this->prepareBox->setRectangleColour(this->app->theme()->popupBG());
        this->prepareBox->setTextColour(this->app->theme()->accent());
        Aether::Element * body = new Aether::Element(0, 0, 700);
        Aether::TextBlock * tips = new Aether::TextBlock(40, 40, "Updating search data, this shouldn't take too long...", 24, 620);
        tips->setColour(this->app->theme()->FG());
        body->addElement(tips);
        body->setH(tips->h() + 80);
        this->prepareBox->setBody(body);
        this->prepareBox->setBodySize(body->w(), body->h());
        this->app->addOverlay(this->prepareBox);
    }

    Search::~Search() {
        delete this->prepareBox;
    }
};