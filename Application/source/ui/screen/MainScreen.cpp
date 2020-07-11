#include "Application.hpp"
#include "Log.hpp"
#include "ui/frame/FrameArtists.hpp"
#include "ui/frame/FrameQueue.hpp"
#include "ui/frame/FrameSongs.hpp"
#include "ui/screen/MainScreen.hpp"
#include "utils/MP3.hpp"

namespace Screen {
    MainScreen::MainScreen(Main::Application * a) : Screen() {
        this->app = a;
        this->playingID = -1;

        // Either exit or unfocus player on B
        this->onButtonPress(Aether::Button::B, [this]() {
            if (this->playerDim->hidden()) {
                this->app->exit();
            } else {
                this->setFocussed(this->container);
            }
        });

        // (Un)focus player when Y pressed
        this->onButtonPress(Aether::Button::Y, [this]() {
            if (this->playerDim->hidden()) {
                this->setFocussed(this->player);
                this->playerDim->setHidden(false);
            } else {
                this->setFocussed(this->container);
            }
        });
    }

    void MainScreen::update(uint32_t dt) {
        // Update the player elements
        PlaybackStatus ps = this->app->sysmodule()->status();
        if (ps == PlaybackStatus::Error) {
            // Handle error
        }
        this->player->setPlaying(ps == PlaybackStatus::Playing);
        this->player->setRepeat(this->app->sysmodule()->repeatMode());
        this->player->setShuffle(this->app->sysmodule()->shuffleMode() == ShuffleMode::On);
        this->player->setPosition(this->app->sysmodule()->position());
        this->player->setVolume(this->app->sysmodule()->volume());

        // Update song metadata
        SongID id = this->app->sysmodule()->currentSong();
        if (id != this->playingID) {
            this->playingID = id;
            Metadata::Song m = this->app->database()->getSongMetadataForID(id);
            if (m.ID != -1) {
                this->player->setTrackName(m.title);
                this->player->setTrackArtist(m.artist);
                this->player->setDuration(m.duration);
            }

            // Change album cover
            std::string path = this->app->database()->getPathForID(id);
            Metadata::AlbumArt art = Utils::MP3::getArtFromID3(path);
            this->player->setAlbumCover(art.data, art.size);
            delete[] art.data;
        }

        // Show/hide dimming element based on current state
        this->playerDim->setHidden(!(this->focussed() == this->player && !this->isTouch));

        // Now update elements
        Screen::update(dt);
    }

    void MainScreen::finalizeState() {
        // Mark the container non-selectable so the highlight won't jump to it
        this->container->addElement(this->frame);
        this->container->setHasSelectable(false);

        // (Re)add dimming element after frame so it's drawn on top
        this->playerDim = new Aether::Rectangle(0, 0, 1280, 590);
        this->playerDim->setColour(Aether::Colour{0, 0, 0, 130});
        this->addElement(this->playerDim);
    }

    void MainScreen::resetState() {
        this->sideRP->setActivated(false);
        this->sideSongs->setActivated(false);
        this->sideArtists->setActivated(false);
        this->sideAlbums->setActivated(false);
        this->sideQueue->setActivated(false);
        this->container->removeElement(this->frame);
        this->removeElement(this->playerDim);
        this->frame = nullptr;
        this->playerDim = nullptr;
    }

    void MainScreen::setupArtists() {
        this->resetState();
        this->sideArtists->setActivated(true);
        this->frame = new Frame::Artists(this->app);
        this->finalizeState();
    }

    void MainScreen::setupQueue() {
        this->resetState();
        this->sideQueue->setActivated(true);
        this->frame = new Frame::Queue(this->app);
        this->finalizeState();
    }

    void MainScreen::setupSongs() {
        this->resetState();
        this->sideSongs->setActivated(true);
        this->frame = new Frame::Songs(this->app);
        this->finalizeState();
    }

    void MainScreen::onLoad() {
        // Add background images
        this->bg = new Aether::Image(0, 0, "romfs:/bg/main.png");
        this->addElement(this->bg);
        this->sidegrad = new Aether::Image(310, 15, "romfs:/misc/sidegradient.png");
        this->sidegrad->setColour(Aether::Colour{255, 255, 255, 200});
        this->addElement(this->sidegrad);
        this->sideBg = new Aether::Rectangle(0, 0, 310, 590);
        this->sideBg->setColour(this->app->theme()->sideBG());
        this->addElement(this->sideBg);

        // Root container for everything except player
        this->container = new Aether::Container(0, 0, 1280, 590);

        // === SIDEBAR ===
        // User
        this->userBg = new Aether::Rectangle(25, 30, 200, 50, 10);
        this->userBg->setColour(this->app->theme()->muted2());
        this->container->addElement(this->userBg);
        this->userIcon = new Aether::Image(this->userBg->x(), this->userBg->y(), "romfs:/user.png");
        this->userIcon->setWH(50, 50);
        this->container->addElement(this->userIcon);
        this->userText = new Aether::Text(this->userIcon->x() + this->userIcon->w() + 8, 0, "Username", 26);
        this->userText->setY(this->userBg->y() + (this->userBg->h() - this->userText->h())/2);
        this->userText->setColour(this->app->theme()->FG());
        this->container->addElement(this->userText);

        // Settings
        this->settingsBg = new Aether::Rectangle(235, 30, 50, 50, 10);
        this->settingsBg->setColour(this->app->theme()->muted2());
        this->container->addElement(this->settingsBg);
        this->settingsIcon = new Aether::Image(this->settingsBg->x() + 10, this->settingsBg->y() + 10, "romfs:/icons/settings.png");
        this->container->addElement(this->settingsIcon);

        // Search bar
        this->search = new CustomElm::SearchBox(25, 130, 260);
        this->search->setBoxColour(this->app->theme()->muted2());
        this->search->setIconColour(this->app->theme()->FG());
        this->container->addElement(this->search);

        // Navigation list
        this->sideRP = new CustomElm::SideButton(10, 210, 290);
        this->sideRP->setIcon(new Aether::Image(0, 0, "romfs:/icons/clock.png"));
        this->sideRP->setText("Recently Played");
        this->sideRP->setActiveColour(this->app->theme()->accent());
        this->sideRP->setInactiveColour(this->app->theme()->FG());
        this->sideRP->setCallback([this](){

        });
        this->container->addElement(this->sideRP);
        this->sideSongs = new CustomElm::SideButton(10, this->sideRP->y() + 60, 290);
        this->sideSongs->setIcon(new Aether::Image(0, 0, "romfs:/icons/musicnote.png"));
        this->sideSongs->setText("Songs");
        this->sideSongs->setActiveColour(this->app->theme()->accent());
        this->sideSongs->setInactiveColour(this->app->theme()->FG());
        this->sideSongs->setCallback([this](){
            this->setupSongs();
        });
        this->container->addElement(this->sideSongs);
        this->sideArtists = new CustomElm::SideButton(10, this->sideSongs->y() + 60, 290);
        this->sideArtists->setIcon(new Aether::Image(0, 0, "romfs:/icons/user.png"));
        this->sideArtists->setText("Artists");
        this->sideArtists->setActiveColour(this->app->theme()->accent());
        this->sideArtists->setInactiveColour(this->app->theme()->FG());
        this->sideArtists->setCallback([this](){
            this->setupArtists();
        });
        this->container->addElement(this->sideArtists);
        this->sideAlbums = new CustomElm::SideButton(10, this->sideArtists->y() + 60, 290);
        this->sideAlbums->setIcon(new Aether::Image(0, 0, "romfs:/icons/disc.png"));
        this->sideAlbums->setText("Albums");
        this->sideAlbums->setActiveColour(this->app->theme()->accent());
        this->sideAlbums->setInactiveColour(this->app->theme()->FG());
        this->sideAlbums->setCallback([this](){

        });
        this->container->addElement(this->sideAlbums);
        this->sideSeparator = new Aether::Rectangle(30, this->sideAlbums->y() + 70, 250, 1);
        this->sideSeparator->setColour(this->app->theme()->muted2());
        this->container->addElement(this->sideSeparator);
        this->sideQueue = new CustomElm::SideButton(10, this->sideSeparator->y() + 10, 290);
        this->sideQueue->setIcon(new Aether::Image(0, 0, "romfs:/icons/queue.png"));
        this->sideQueue->setText("Play Queue");
        this->sideQueue->setActiveColour(this->app->theme()->accent());
        this->sideQueue->setInactiveColour(this->app->theme()->FG());
        this->sideQueue->setCallback([this](){
            this->setupQueue();
        });
        this->container->addElement(this->sideQueue);
        this->addElement(this->container);

        // === PLAYER ===
        this->player = new CustomElm::Player();
        this->player->setAccentColour(this->app->theme()->accent());
        this->player->setBackgroundColour(this->app->theme()->bottomBG());
        this->player->setForegroundColour(this->app->theme()->FG());
        this->player->setMutedColour(this->app->theme()->muted());
        this->player->setMuted2Colour(this->app->theme()->muted2());
        this->player->setShuffleCallback([this]() {
            this->app->sysmodule()->sendSetShuffle((this->app->sysmodule()->shuffleMode() == ShuffleMode::Off ? ShuffleMode::On : ShuffleMode::Off));
        });
        this->player->setPreviousCallback([this]() {
            this->app->sysmodule()->sendPrevious();
        });
        this->player->setPauseCallback([this]() {
            this->app->sysmodule()->sendPause();
        });
        this->player->setPlayCallback([this]() {
            this->app->sysmodule()->sendResume();
        });
        this->player->setNextCallback([this]() {
            this->app->sysmodule()->sendNext();
        });
        this->player->setRepeatCallback([this](RepeatMode rm) {
            this->app->sysmodule()->sendSetRepeat(rm);
        });
        this->player->setSeekCallback([this](float f) {
            this->app->sysmodule()->sendSetPosition(f);
        });
        this->player->setVolumeIconCallback([this](bool muteIcon) {
            if (muteIcon) {
                this->app->sysmodule()->sendUnmute();
            } else {
                this->app->sysmodule()->sendMute();
            }
        });
        this->player->setVolumeCallback([this](float f) {
            this->app->sysmodule()->sendSetVolume(f);
        });
        this->player->setFullscreenCallback([this]() {
            // Change screen
        });
        this->addElement(this->player);
        this->player->setHasSelectable(false);

        // Set songs active
        this->frame = nullptr;
        this->playerDim = nullptr;
        this->setupSongs();
        this->container->setFocussed(this->sideSongs);
    }

    void MainScreen::onUnload() {
        this->resetState();
        this->removeElement(this->container);
        this->removeElement(this->player);
    }
};