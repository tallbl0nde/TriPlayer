#include "Application.hpp"
#include "ui/frame/Albums.hpp"
#include "ui/frame/Artist.hpp"
#include "ui/frame/Artists.hpp"
#include "ui/frame/ArtistInfo.hpp"
#include "ui/frame/Queue.hpp"
#include "ui/frame/Songs.hpp"
#include "ui/screen/Home.hpp"
#include "utils/MP3.hpp"

#include "ui/overlay/FileBrowser.hpp"

namespace Screen {
    Home::Home(Main::Application * a) : Screen() {
        this->app = a;
        this->playingID = -1;

        // Attempt the following in order when B is pressed:
        // 1. Shift focus from the player to the main frame
        // 2. Delete the current frame and pop one from the stack
        // 3. Show exit prompt
        this->onButtonPress(Aether::Button::B, [this]() {
            if (this->focussed() == this->player) {
                this->setFocussed(this->container);

            } else if (!this->frameStack.empty()) {
                this->backCallback();

            } else {
                // I'll put a prompt here one day
                this->app->exit();
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

        // Create dimming element
        this->playerDim = new Aether::Rectangle(0, 0, 1280, 590);
        this->playerDim->setColour(Aether::Colour{0, 0, 0, 130});
    }

    void Home::backCallback() {
        if (!this->frameStack.empty()) {
            this->container->removeElement(this->frame);
            this->frame = this->frameStack.top();
            this->frameStack.pop();

            this->container->addElement(this->frame);
            this->container->setHasSelectable(false);
            this->returnElement(this->playerDim);
            this->addElement(this->playerDim);
        }
    }

    void Home::changeFrame(Frame::Type t, Frame::Action a, int id) {
        switch (a) {
            // Push the current frame on the stack
            case Frame::Action::Push:
                // Maybe show an error if too deep?
                this->container->returnElement(this->frame);
                this->frameStack.push(this->frame);
                this->returnElement(this->playerDim);
                break;

            // Empty stack and delete current frame
            case Frame::Action::Reset:
                this->container->removeElement(this->frame);
                while (!this->frameStack.empty()) {
                    delete this->frameStack.top();
                    this->frameStack.pop();
                }
                this->resetState();
                break;
        }
        this->frame = nullptr;

        switch (t) {
            case Frame::Type::Playlists:
                // this->sidePlaylists->setActivated(true);
                // this->frame = new Frame::Playlists(this->app);
                break;

            case Frame::Type::Playlist:
                // this->frame = new Frame::Playlist(this->app, id);
                break;

            case Frame::Type::Albums:
                this->sideAlbums->setActivated(true);
                this->frame = new Frame::Albums(this->app);
                break;

            case Frame::Type::Album:
                // this->frame = new Frame::Album(this->app, id);
                break;

            case Frame::Type::Artists:
                this->sideArtists->setActivated(true);
                this->frame = new Frame::Artists(this->app);
                break;

            case Frame::Type::Artist:
                this->frame = new Frame::Artist(this->app, id);
                break;

            case Frame::Type::ArtistInfo:
                this->frame = new Frame::ArtistInfo(this->app, id);
                break;

            case Frame::Type::Songs:
                this->sideSongs->setActivated(true);
                this->frame = new Frame::Songs(this->app);
                break;

            case Frame::Type::Queue:
                this->sideQueue->setActivated(true);
                this->frame = new Frame::Queue(this->app);
                break;
        }
        this->frame->setChangeFrameFunc([this](Frame::Type t, Frame::Action a, int id) {
            this->changeFrame(t, a, id);
        });
        this->finalizeState();
    }

    void Home::update(uint32_t dt) {
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
            Metadata::Art art = Utils::MP3::getArtFromID3(path);
            this->player->setAlbumCover(art.data, art.size);
            delete[] art.data;
        }

        // Show/hide dimming element based on current state
        this->playerDim->setHidden(!(this->focussed() == this->player && !this->isTouch));

        // Set back button colour and behaviour based on the stack
        if (this->frameStack.empty()) {
            this->backIcon->setColour(this->app->theme()->muted());
            this->backText->setColour(this->app->theme()->muted());
            this->backButton->setTouchable(false);

        } else {
            this->backIcon->setColour(this->app->theme()->FG());
            this->backText->setColour(this->app->theme()->FG());
            this->backButton->setTouchable(true);
        }

        // Now update elements
        Screen::update(dt);
    }

    void Home::finalizeState() {
        // Mark the container non-selectable so the highlight won't jump to it
        this->container->addElement(this->frame);
        this->container->setHasSelectable(false);

        // (Re)add dimming element after frame so it's drawn on top
        this->addElement(this->playerDim);
    }

    void Home::resetState() {
        // Deselect all side buttons
        this->sideRP->setActivated(false);
        this->sideSongs->setActivated(false);
        this->sideArtists->setActivated(false);
        this->sideAlbums->setActivated(false);
        this->sideQueue->setActivated(false);

        // Get the dimming element back so it can be added afterwards again
        this->returnElement(this->playerDim);
    }

    void Home::onLoad() {
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
        this->sideContainer = new Aether::Container(0, 0, 310, 590);

        // Navigation outlines
        Aether::Rectangle * r = new Aether::Rectangle(15, 65, 280, 1);
        r->setColour(this->app->theme()->muted2());
        this->sideContainer->addElement(r);
        r = new Aether::Rectangle(155, 10, 1, 45);
        r->setColour(this->app->theme()->muted2());
        this->sideContainer->addElement(r);

        // Back
        this->backButton = new Aether::Element(0, 0, 155, 65);
        this->backIcon = new Aether::Image(this->backButton->x() + 30, 17, "romfs:/icons/back.png");
        this->backIcon->setColour(this->app->theme()->FG());
        this->backButton->addElement(this->backIcon);
        this->backText = new Aether::Text(this->backIcon->x() + this->backIcon->w() + 20, 32, "Back", 26);
        this->backText->setY(this->backText->y() - this->backText->h()/2);
        this->backText->setColour(this->app->theme()->FG());
        this->backButton->addElement(this->backText);
        this->backButton->setCallback([this]() {
            this->backCallback();
        });
        this->backButton->setSelectable(false);
        this->sideContainer->addElement(this->backButton);

        // Quit
        Aether::Element * quitButton = new Aether::Element(155, 0, 155, 65);
        Aether::Text * quitText = new Aether::Text(quitButton->x() + 30, 32, "Quit", 26);
        quitText->setY(quitText->y() - quitText->h()/2);
        quitText->setColour(this->app->theme()->FG());
        quitButton->addElement(quitText);
        Aether::Image * quitIcon = new Aether::Image(quitText->x() + quitText->w() + 20, 22, "romfs:/icons/quit.png");
        quitIcon->setColour(this->app->theme()->FG());
        quitButton->addElement(quitIcon);
        quitButton->setCallback([this]() {
            // Have a prompt
            this->app->exit();
        });
        quitButton->setSelectable(false);
        this->sideContainer->addElement(quitButton);

        // User
        this->userBg = new Aether::Rectangle(25, 90, 200, 50, 10);
        this->userBg->setColour(this->app->theme()->muted2());
        this->sideContainer->addElement(this->userBg);
        this->userIcon = new Aether::Image(this->userBg->x(), this->userBg->y(), "romfs:/user.png");
        this->userIcon->setWH(50, 50);
        this->sideContainer->addElement(this->userIcon);
        this->userText = new Aether::Text(this->userIcon->x() + this->userIcon->w() + 8, 0, "Username", 26);
        this->userText->setY(this->userBg->y() + (this->userBg->h() - this->userText->h())/2);
        this->userText->setColour(this->app->theme()->FG());
        this->sideContainer->addElement(this->userText);

        // Settings
        this->settingsBg = new Aether::Rectangle(235, 90, 50, 50, 10);
        this->settingsBg->setColour(this->app->theme()->muted2());
        this->sideContainer->addElement(this->settingsBg);
        this->settingsIcon = new Aether::Image(this->settingsBg->x() + 10, this->settingsBg->y() + 10, "romfs:/icons/settings.png");
        this->sideContainer->addElement(this->settingsIcon);

        // Search bar
        this->search = new CustomElm::SearchBox(25, 170, 260);
        this->search->setBoxColour(this->app->theme()->muted2());
        this->search->setIconColour(this->app->theme()->FG());
        this->sideContainer->addElement(this->search);

        // Navigation list
        this->sideRP = new CustomElm::SideButton(10, 230, 290);
        this->sideRP->setIcon(new Aether::Image(0, 0, "romfs:/icons/clock.png"));
        this->sideRP->setText("Recently Played");
        this->sideRP->setActiveColour(this->app->theme()->accent());
        this->sideRP->setInactiveColour(this->app->theme()->FG());
        this->sideRP->setCallback([this](){

        });
        this->sideContainer->addElement(this->sideRP);
        this->sideSongs = new CustomElm::SideButton(10, this->sideRP->y() + 60, 290);
        this->sideSongs->setIcon(new Aether::Image(0, 0, "romfs:/icons/musicnote.png"));
        this->sideSongs->setText("Songs");
        this->sideSongs->setActiveColour(this->app->theme()->accent());
        this->sideSongs->setInactiveColour(this->app->theme()->FG());
        this->sideSongs->setCallback([this](){
            this->changeFrame(Frame::Type::Songs, Frame::Action::Reset);
        });
        this->sideContainer->addElement(this->sideSongs);
        this->sideArtists = new CustomElm::SideButton(10, this->sideSongs->y() + 60, 290);
        this->sideArtists->setIcon(new Aether::Image(0, 0, "romfs:/icons/user.png"));
        this->sideArtists->setText("Artists");
        this->sideArtists->setActiveColour(this->app->theme()->accent());
        this->sideArtists->setInactiveColour(this->app->theme()->FG());
        this->sideArtists->setCallback([this](){
            this->changeFrame(Frame::Type::Artists, Frame::Action::Reset);
        });
        this->sideContainer->addElement(this->sideArtists);
        this->sideAlbums = new CustomElm::SideButton(10, this->sideArtists->y() + 60, 290);
        this->sideAlbums->setIcon(new Aether::Image(0, 0, "romfs:/icons/disc.png"));
        this->sideAlbums->setText("Albums");
        this->sideAlbums->setActiveColour(this->app->theme()->accent());
        this->sideAlbums->setInactiveColour(this->app->theme()->FG());
        this->sideAlbums->setCallback([this](){
            this->changeFrame(Frame::Type::Albums, Frame::Action::Reset);
        });
        this->sideContainer->addElement(this->sideAlbums);
        this->sideSeparator = new Aether::Rectangle(30, this->sideAlbums->y() + 70, 250, 1);
        this->sideSeparator->setColour(this->app->theme()->muted2());
        this->sideContainer->addElement(this->sideSeparator);
        this->sideQueue = new CustomElm::SideButton(10, this->sideSeparator->y() + 10, 290);
        this->sideQueue->setIcon(new Aether::Image(0, 0, "romfs:/icons/queue.png"));
        this->sideQueue->setText("Play Queue");
        this->sideQueue->setActiveColour(this->app->theme()->accent());
        this->sideQueue->setInactiveColour(this->app->theme()->FG());
        this->sideQueue->setCallback([this](){
            this->changeFrame(Frame::Type::Queue, Frame::Action::Reset);
        });
        this->sideContainer->addElement(this->sideQueue);
        this->sideContainer->setFocussed(this->sideSongs);
        this->container->addElement(this->sideContainer);
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
        this->changeFrame(Frame::Type::Songs, Frame::Action::Reset);
        this->container->setFocussed(this->frame);
    }

    void Home::onUnload() {
        // Ensure all frames are deleted
        this->container->removeElement(this->frame);
        while (!this->frameStack.empty()) {
            delete this->frameStack.top();
            this->frameStack.pop();
        }

        // The rest of this isn't necessary in this context but it's good to do so
        this->removeElement(this->playerDim);
        this->removeElement(this->player);
        this->removeElement(this->container);
    }
};