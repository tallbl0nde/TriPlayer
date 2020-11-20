#include "Application.hpp"
#include "lang/Lang.hpp"
#include "ui/frame/settings/AppSearch.hpp"

namespace Frame::Settings {
    AppSearch::AppSearch(Main::Application * a) : Frame(a) {
        // Temporary variables
        Config * cfg = this->app->config();
        Aether::ListOption * opt;

        // Search::max_playlists
        opt = new Aether::ListOption("Settings.AppSearch.PlaylistLimit"_lang, std::to_string(cfg->searchMaxPlaylists()), nullptr);
        opt->setCallback([this, cfg, opt]() {
            int val = cfg->searchMaxPlaylists();
            if (this->getNumberInput(val, "Settings.AppSearch.PlaylistLimit"_lang, "", true)) {
                val = (val < -1 ? -1 : val);
                if (cfg->setSearchMaxPlaylists(val)) {
                    opt->setValue(std::to_string(val));
                }
            }
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), this->app->theme()->accent());
        this->list->addElement(opt);

        // Search::max_albums
        opt = new Aether::ListOption("Settings.AppSearch.AlbumLimit"_lang, std::to_string(cfg->searchMaxAlbums()), nullptr);
        opt->setCallback([this, cfg, opt]() {
            int val = cfg->searchMaxAlbums();
            if (this->getNumberInput(val, "Settings.AppSearch.AlbumLimit"_lang, "", true)) {
                val = (val < -1 ? -1 : val);
                if (cfg->setSearchMaxAlbums(val)) {
                    opt->setValue(std::to_string(val));
                }
            }
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), this->app->theme()->accent());
        this->list->addElement(opt);

        // Search::max_artists
        opt = new Aether::ListOption("Settings.AppSearch.ArtistLimit"_lang, std::to_string(cfg->searchMaxArtists()), nullptr);
        opt->setCallback([this, cfg, opt]() {
            int val = cfg->searchMaxArtists();
            if (this->getNumberInput(val, "Settings.AppSearch.ArtistLimit"_lang, "", true)) {
                val = (val < -1 ? -1 : val);
                if (cfg->setSearchMaxArtists(val)) {
                    opt->setValue(std::to_string(val));
                }
            }
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), this->app->theme()->accent());
        this->list->addElement(opt);

        // Search::max_songs
        opt = new Aether::ListOption("Settings.AppSearch.SongLimit"_lang, std::to_string(cfg->searchMaxSongs()), nullptr);
        opt->setCallback([this, cfg, opt]() {
            int val = cfg->searchMaxSongs();
            if (this->getNumberInput(val, "Settings.AppSearch.SongLimit"_lang, "", true)) {
                val = (val < -1 ? -1 : val);
                if (cfg->setSearchMaxSongs(val)) {
                    opt->setValue(std::to_string(val));
                }
            }
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), this->app->theme()->accent());
        this->list->addElement(opt);

        // Comment
        this->addComment("Settings.AppSearch.Comment"_lang);
        this->list->addElement(new Aether::ListSeparator());
    }
};