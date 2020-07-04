#include "Application.hpp"
#include "MainScreen.hpp"
#include "MP3.hpp"
#include "Splash.hpp"

namespace Main {
    Application::Application() {
        // Prepare theme
        this->theme_ = new Theme();

        // Open database
        this->database_ = new Database();
        this->database_->migrate();

        // Create sysmodule object (will attempt connection)
        this->sysmodule_ = new Sysmodule();
        // Continue in another thread
        this->sysThread = std::async(std::launch::async, &Sysmodule::process, this->sysmodule_);

        // Create Aether instance
        Aether::ThreadPool::setMaxThreads(8);
        this->display = new Aether::Display();
        this->display->setBackgroundColour(0, 0, 0);
        this->display->setFont("romfs:/Quicksand.ttf");
        this->display->setHighlightColours(Aether::Colour{255, 255, 255, 0}, this->theme_->selected());
        this->display->setHighlightAnimation(Aether::Theme::Dark.highlightFunc);
        this->display->setFadeIn();
        this->display->setShowFPS(true);

        // Setup screens
        this->scSplash = new Screen::Splash(this);
        this->scMain = new Screen::MainScreen(this);
        this->setScreen(ScreenID::Splash);

        // Create overlays
        this->ovlSongMenu = nullptr;
    }

    void Application::setHoldDelay(int i) {
        this->display->setHoldDelay(i);
    }

    void Application::addOverlay(Aether::Overlay * o) {
        this->display->addOverlay(o);
    }

    void Application::setupSongMenu(SongID id) {
        // Appearance
        this->ovlSongMenu->setRemoveFromQueueText("Remove from Queue");
        this->ovlSongMenu->setAddToQueueText("Add to Queue");
        this->ovlSongMenu->setAddToPlaylistText("Add to Playlist");
        this->ovlSongMenu->setGoToArtistText("Go to Artist");
        this->ovlSongMenu->setGoToAlbumText("Go to Album");
        this->ovlSongMenu->setViewDetailsText("View Details");
        this->ovlSongMenu->setBackgroundColour(this->theme_->popupBG());
        this->ovlSongMenu->setIconColour(this->theme_->muted());
        this->ovlSongMenu->setLineColour(this->theme_->muted2());
        this->ovlSongMenu->setMutedTextColour(this->theme_->muted());
        this->ovlSongMenu->setTextColour(this->theme_->FG());

        // Song Metadata
        SongInfo si = this->database_->getSongInfoForID(id);
        this->ovlSongMenu->setTitle(si.title);
        this->ovlSongMenu->setArtist(si.artist);

        // Song Art
        bool hasArt = false;
        std::string path = this->database_->getPathForID(id);
        if (path.length() > 0) {
            SongArt sa = Utils::MP3::getArtFromID3(path);
            if (sa.data != nullptr) {
                this->ovlSongMenu->setAlbum(new Aether::Image(0, 0, sa.data, sa.size));
                hasArt = true;
                delete[] sa.data;
            }
        }
        if (!hasArt) {
            this->ovlSongMenu->setAlbum(new Aether::Image(0, 0, "romfs:/misc/noalbum.png"));
        }

        // Callbacks
        this->ovlSongMenu->setAddToQueueFunc([this, id]() {
            this->sysmodule_->sendAddToSubQueue(id);
            this->ovlSongMenu->close();
        });
        this->ovlSongMenu->setAddToPlaylistFunc([this, id]() {
            // add to playlist
        });
        this->ovlSongMenu->setGoToArtistFunc([this, id]() {
            // go to artist
        });
        this->ovlSongMenu->setGoToAlbumFunc([this, id]() {
            // go to album
        });
        this->ovlSongMenu->setViewDetailsFunc([this, id]() {
            // view details
        });
    }

    void Application::showSongMenu(SongID id) {
        delete this->ovlSongMenu;
        this->ovlSongMenu = new CustomOvl::SongMenu(false);
        this->setupSongMenu(id);
        this->addOverlay(this->ovlSongMenu);
    }

    void Application::showSongMenu(SongID id, size_t pos, SongMenuType type) {
        delete this->ovlSongMenu;
        this->ovlSongMenu = new CustomOvl::SongMenu(true);
        this->setupSongMenu(id);
        if (type == SongMenuType::RemoveFromQueue) {
            this->ovlSongMenu->setRemoveFromQueueFunc([this, pos]() {
                this->sysmodule_->sendRemoveFromQueue(pos);
                this->ovlSongMenu->close();
            });

        } else {
            this->ovlSongMenu->setRemoveFromQueueFunc([this, pos]() {
                this->sysmodule_->sendRemoveFromSubQueue(pos);
                this->ovlSongMenu->close();
            });
        }
        this->addOverlay(this->ovlSongMenu);
    }

    void Application::setScreen(ScreenID s) {
        switch (s) {
            case Splash:
                this->display->setScreen(this->scSplash);
                break;

            case Main:
                this->display->setScreen(this->scMain);
                break;
        }
    }

    void Application::pushScreen() {
        this->display->pushScreen();
    }

    void Application::popScreen() {
        this->display->popScreen();
    }

    Database * Application::database() {
        return this->database_;
    }

    Sysmodule * Application::sysmodule() {
        return this->sysmodule_;
    }

    Theme * Application::theme() {
        return this->theme_;
    }

    void Application::run() {
        // Do main loop
        while (this->display->loop()) {

        }
    }

    void Application::exit() {
        this->display->exit();
    }

    Application::~Application() {
        // Delete overlays
        delete this->ovlSongMenu;

        // Delete screens
        delete this->scMain;
        delete this->scSplash;

        // Cleanup Aether after screens are deleted
        delete this->display;

        // Disconnect from sysmodule
        this->sysmodule_->exit();
        this->sysThread.get();
        delete this->sysmodule_;

        // Close/save database
        delete this->database_;
    }
};