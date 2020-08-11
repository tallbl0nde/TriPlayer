#include "Application.hpp"
#include "ui/element/ListHeadingCount.hpp"
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
            return;
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
    }

    void Search::addSongs(const std::vector<Metadata::Song> & v) {
        // Add heading and count first
        CustomElm::ListHeadingCount * heading = new CustomElm::ListHeadingCount();
        heading->setHeadingString("Songs");
        heading->setCount(v.size());
        heading->setTextColour(this->app->theme()->FG());
        this->list->addElement(heading);
        if (this->listEmpty) {
            heading->setY(this->list->y() + 10);
            this->listEmpty = false;
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
            this->addSongs(songs);
        }

        // Copy song IDs to set queue if playing a song
        for (size_t i = 0; i < songs.size(); i++) {
            this->songIDs.push_back(songs[i].ID);
        }
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

    Search::~Search() {

    }
};