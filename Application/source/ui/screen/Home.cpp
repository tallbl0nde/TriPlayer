#include "Application.hpp"
#include "Paths.hpp"
#include "ui/frame/Album.hpp"
#include "ui/frame/Albums.hpp"
#include "ui/frame/AlbumInfo.hpp"
#include "ui/frame/Artist.hpp"
#include "ui/frame/Artists.hpp"
#include "ui/frame/ArtistInfo.hpp"
#include "ui/frame/Playlist.hpp"
#include "ui/frame/Playlists.hpp"
#include "ui/frame/PlaylistInfo.hpp"
#include "ui/frame/Queue.hpp"
#include "ui/frame/Search.hpp"
#include "ui/frame/Songs.hpp"
#include "ui/frame/SongInfo.hpp"
#include "ui/screen/Home.hpp"
#include "utils/Random.hpp"

namespace Screen {
    Home::Home(Main::Application * a) : Screen(a) {
        // Attempt the following in order when B is pressed:
        // 1. Shift focus from the player to the main frame
        // 2. Delete the current frame and pop one from the stack
        // 3. (Attempt to) exit the app
        this->onButtonPress(Aether::Button::B, [this]() {
            if (this->focussed() == this->player) {
                this->setFocussed(this->container);

            } else if (!this->frameStack.empty()) {
                this->backCallback();

            } else {
                this->app->exit(false);
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

        // Use ZL/ZR to scroll through lists faster
        this->onButtonPress(Aether::Button::ZL, [this](){
            this->app->setHoldDelay(20);
        });
        this->onButtonRelease(Aether::Button::ZL, [this](){
            this->app->setHoldDelay(100);
        });
        this->onButtonPress(Aether::Button::ZR, [this](){
            this->app->setHoldDelay(20);
        });
        this->onButtonRelease(Aether::Button::ZR, [this](){
            this->app->setHoldDelay(100);
        });
    }

    void Home::backCallback() {
        if (!this->frameStack.empty()) {
            // Delete the current frame and pop one from the stack
            this->resetState();
            this->container->removeElement(this->frame);
            FrameTuple tuple = this->frameStack.top();
            this->frameStack.pop();
            this->frame = tuple.frame;
            this->frameType = tuple.type;
            this->frame->onPop(tuple.pushedType);
            this->finalizeState();
        }
    }

    void Home::showAddToPlaylist(std::function<void(PlaylistID)> f) {
        // Always recreate menu
        delete this->addToPlMenu;
        this->addToPlMenu = new CustomOvl::AddToPlaylist();
        this->addToPlMenu->setBackgroundColour(this->app->theme()->popupBG());
        this->addToPlMenu->setHeadingColour(this->app->theme()->FG());
        this->addToPlMenu->setHeadingString("Common.AddToPlaylist"_lang);
        this->addToPlMenu->setLineColour(this->app->theme()->muted2());
        this->addToPlMenu->setChosenCallback([this, f](PlaylistID id) {
            this->app->lockDatabase();
            f(id);
            this->app->unlockDatabase();

            // Close menu if playlist chosen
            if (id >= 0) {
                this->addToPlMenu->close();
            }
        });

        // Insert items for playlists
        std::vector<Metadata::Playlist> pls = this->app->database()->getAllPlaylistMetadata(Database::SortBy::TitleAsc);
        for (size_t i = 0; i < pls.size(); i++) {
            CustomElm::ListItem::Playlist * l = new CustomElm::ListItem::Playlist(pls[i].imagePath.empty() ? "romfs:/misc/noplaylist.png" : pls[i].imagePath);
            l->setNameString(pls[i].name);
            std::string str = (pls[i].songCount == 1 ? "Common.Song" : Utils::regexReplace("Common.Songs"_lang, std::to_string(pls[i].songCount)));
            l->setSongsString(str);
            l->setLineColour(this->app->theme()->muted2());
            l->setMoreColour(this->app->theme()->muted());
            l->setTextColour(this->app->theme()->FG());
            l->setMutedTextColour(this->app->theme()->muted());
            this->addToPlMenu->addPlaylist(l, pls[i].ID);
        }

        // Finally show menu
        this->app->addOverlay(this->addToPlMenu);
    }

    void Home::showConfirmQueue(const std::string & str, const std::vector<SongID> & ids, const size_t pos, const bool rand) {
        // Always recreate prompt
        delete this->confirmQueue;
        this->confirmQueue = new Aether::MessageBox();
        this->confirmQueue->setLineColour(this->app->theme()->muted2());
        this->confirmQueue->setRectangleColour(this->app->theme()->popupBG());
        this->confirmQueue->addLeftButton("Common.Cancel"_lang, [this]() {
            this->confirmQueue->close();
        });
        this->confirmQueue->addRightButton("Common.OK"_lang, [this, str, ids, pos, rand]() {
            this->confirmQueue->close();

            // "Shuffle" locally first if requested
            std::vector<SongID> vec;
            if (rand && this->app->sysmodule()->shuffleMode() == ShuffleMode::On) {
                vec = ids;
                size_t pos = Utils::Random::getSizeT(0, ids.size()-1);
                SongID tmp = vec[0];
                vec[0] = vec[pos];
                vec[pos] = tmp;
            }

            // Send queue
            this->app->sysmodule()->sendSetPlayingFrom(str);
            this->app->sysmodule()->sendSetQueue((vec.empty() ? ids : vec));
            this->app->sysmodule()->sendSetSongIdx(pos);
            this->app->sysmodule()->sendSetShuffle(this->app->sysmodule()->shuffleMode());
        });
        this->confirmQueue->setTextColour(this->app->theme()->accent());
        Aether::Element * body = new Aether::Element(0, 0, 700);
        Aether::TextBlock * tips = new Aether::TextBlock(40, 40, "Common.ClearQueue"_lang, 24, 620);
        tips->setColour(this->app->theme()->FG());
        body->addElement(tips);
        body->setH(tips->h() + 80);
        this->confirmQueue->setBody(body);
        this->confirmQueue->setBodySize(body->w(), body->h());
        this->app->addOverlay(this->confirmQueue);
    }

    void Home::changeFrame(Frame::Type t, Frame::Action a, int id) {
        switch (a) {
            // Mark that we should move back a frame
            case Frame::Action::Back:
                this->backOneFrame++;
                return;
                break;

            // Push the current frame on the stack
            case Frame::Action::Push:
                // Maybe show an error if too deep?
                this->frame->onPush(t);
                this->container->returnElement(this->frame);
                this->frameStack.push(FrameTuple{this->frame, this->frameType, t});
                break;

            // Empty stack and delete current frame
            case Frame::Action::Reset:
                this->container->removeElement(this->frame);
                while (!this->frameStack.empty()) {
                    FrameTuple tuple = this->frameStack.top();
                    this->frameStack.pop();
                    tuple.frame->onPop(Frame::Type::None);
                    delete tuple.frame;
                }
                break;
        }
        this->frame = nullptr;
        this->resetState();

        switch (t) {
            case Frame::Type::Playlists:
                this->frame = new Frame::Playlists(this->app);
                break;

            case Frame::Type::Playlist:
                this->frame = new Frame::Playlist(this->app, id);
                break;

            case Frame::Type::PlaylistInfo:
                this->frame = new Frame::PlaylistInfo(this->app, id);
                break;

            case Frame::Type::Albums:
                this->frame = new Frame::Albums(this->app);
                break;

            case Frame::Type::Album:
                this->frame = new Frame::Album(this->app, id);
                break;

            case Frame::Type::AlbumInfo:
                this->frame = new Frame::AlbumInfo(this->app, id);
                break;

            case Frame::Type::Artists:
                this->frame = new Frame::Artists(this->app);
                break;

            case Frame::Type::Artist:
                this->frame = new Frame::Artist(this->app, id);
                break;

            case Frame::Type::ArtistInfo:
                this->frame = new Frame::ArtistInfo(this->app, id);
                break;

            case Frame::Type::Search:
                this->frame = new Frame::Search(this->app);
                break;

            case Frame::Type::Songs:
                this->frame = new Frame::Songs(this->app);
                break;

            case Frame::Type::SongInfo:
                this->frame = new Frame::SongInfo(this->app, id);
                break;

            case Frame::Type::Queue:
                this->frame = new Frame::Queue(this->app);
                break;

            default:
                break;
        }
        this->frameType = t;
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
            bool updated = false;
            if (id > -1) {
                Metadata::Song m = this->app->database()->getSongMetadataForID(id);

                // Repeated check as ID is set negative on error
                if (m.ID > -1) {
                    this->player->setTrackName(m.title);
                    this->player->setTrackArtist(m.artist);
                    this->player->setDuration(m.duration);
                    AlbumID id = this->app->database()->getAlbumIDForSong(m.ID);
                    Metadata::Album md = this->app->database()->getAlbumMetadataForID(id);
                    this->player->setAlbumCover(new Aether::Image(0, 0, md.imagePath.empty() ? Path::App::DefaultArtFile : md.imagePath));
                    updated = true;
                }
            }

            // Use default text if an error occurs or the ID is negative
            if (!updated) {
                this->player->setTrackName("Common.NotPlaying1"_lang);
                this->player->setTrackArtist("Common.NotPlaying2"_lang);
                this->player->setDuration(0);
                this->player->setAlbumCover(new Aether::Image(0, 0, Path::App::DefaultArtFile));
            }
        }

        // Show/hide dimming element based on current state
        this->playerDim->setHidden(!(this->focussed() == this->player && !this->isTouch));

        // Change frame if needed
        while (this->backOneFrame > 0) {
            this->backCallback();
            this->backOneFrame--;
        }

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

        // Show/hide touch section based on config value
        this->touchContainer->setHidden(!this->app->config()->showTouchControls());
        this->sideContainer->setY(this->app->config()->showTouchControls() ? 0 : -65);
        this->updateDot->setHidden(!this->app->hasUpdate());

        // Now update elements
        Screen::update(dt);
    }

    void Home::updateColours() {
        if (!this->isLoaded) {
            return;
        }

        // Update screen colours
        this->sideSearch->setActiveColour(this->app->theme()->accent());
        this->sidePlaylists->setActiveColour(this->app->theme()->accent());
        this->sideSongs->setActiveColour(this->app->theme()->accent());
        this->sideArtists->setActiveColour(this->app->theme()->accent());
        this->sideAlbums->setActiveColour(this->app->theme()->accent());
        this->sideQueue->setActiveColour(this->app->theme()->accent());
        this->sideSettings->setActiveColour(this->app->theme()->accent());
        this->updateDot->setColour(this->app->theme()->accent());
        this->player->setAccentColour(this->app->theme()->accent());

        // Now also update current frame and all on stack
        this->frame->updateColours();
        std::stack<FrameTuple> tmp;
        while (!this->frameStack.empty()) {
            tmp.push(this->frameStack.top());
            this->frameStack.pop();
        }

        while (!tmp.empty()) {
            this->frameStack.push(tmp.top());
            tmp.top().frame->updateColours();
            tmp.pop();
        }
    }

    void Home::finalizeState() {
        // Set the frame's callbacks
        this->frame->setChangeFrameFunc([this](Frame::Type t, Frame::Action a, int id) {
            this->changeFrame(t, a, id);
        });
        this->frame->setPlayNewQueueFunc([this](const std::string & str, const std::vector<SongID> & ids, const size_t pos, const bool rand) {
            if (this->app->config()->confirmClearQueue() && !this->app->sysmodule()->queue().empty()) {
                this->showConfirmQueue(str, ids, pos, rand);
            } else {
                // "Shuffle" locally first if requested
                std::vector<SongID> vec;
                if (rand && this->app->sysmodule()->shuffleMode() == ShuffleMode::On) {
                    vec = ids;
                    size_t pos = Utils::Random::getSizeT(0, ids.size()-1);
                    SongID tmp = vec[0];
                    vec[0] = vec[pos];
                    vec[pos] = tmp;
                }

                // Send queue
                this->app->sysmodule()->sendSetPlayingFrom(str);
                this->app->sysmodule()->sendSetQueue((vec.empty() ? ids : vec));
                this->app->sysmodule()->sendSetSongIdx(pos);
                this->app->sysmodule()->sendSetShuffle(this->app->sysmodule()->shuffleMode());
            }
        });
        this->frame->setShowAddToPlaylistFunc([this](std::function<void(PlaylistID)> f) {
            this->showAddToPlaylist(f);
        });

        // Mark the container non-selectable so the highlight won't jump to it
        this->container->addElement(this->frame);
        this->container->setHasSelectable(false);

        // (Re)add dimming element after frame so it's drawn on top
        this->addElement(this->playerDim);

        // Highlight appropriate menu entry
        switch (this->frameType) {
            case Frame::Type::Search:
                this->sideSearch->setActivated(true);
                break;

            case Frame::Type::Playlists:
            case Frame::Type::Playlist:
            case Frame::Type::PlaylistInfo:
                this->sidePlaylists->setActivated(true);
                break;

            case Frame::Type::Songs:
            case Frame::Type::SongInfo:
                this->sideSongs->setActivated(true);
                break;

            case Frame::Type::Artists:
            case Frame::Type::Artist:
            case Frame::Type::ArtistInfo:
                this->sideArtists->setActivated(true);
                break;

            case Frame::Type::Albums:
            case Frame::Type::Album:
            case Frame::Type::AlbumInfo:
                this->sideAlbums->setActivated(true);
                break;

            case Frame::Type::Queue:
                this->sideQueue->setActivated(true);
                break;

            default:
                break;
        }
    }

    void Home::resetState() {
        // Deselect all side buttons
        this->sideSearch->setActivated(false);
        this->sidePlaylists->setActivated(false);
        this->sideSongs->setActivated(false);
        this->sideArtists->setActivated(false);
        this->sideAlbums->setActivated(false);
        this->sideQueue->setActivated(false);

        // Get the dimming element back so it can be added afterwards again
        this->returnElement(this->playerDim);
    }

    void Home::onLoad() {
        Screen::onLoad();

        // Create dimming element
        this->playerDim = new Aether::Rectangle(0, 0, 1280, 590);
        this->playerDim->setColour(Aether::Colour{0, 0, 0, 130});

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
        this->touchContainer = new Aether::Container(this->sideContainer->x(), this->sideContainer->y(), this->sideContainer->w(), 65);

        // Navigation outlines
        Aether::Rectangle * r = new Aether::Rectangle(15, 65, 280, 1);
        r->setColour(this->app->theme()->muted2());
        this->touchContainer->addElement(r);
        r = new Aether::Rectangle(155, 10, 1, 45);
        r->setColour(this->app->theme()->muted2());
        this->touchContainer->addElement(r);

        // Back
        this->backButton = new Aether::Element(0, 0, 155, 65);
        this->backIcon = new Aether::Image(this->backButton->x() + 30, 17, "romfs:/icons/back.png");
        this->backIcon->setColour(this->app->theme()->FG());
        this->backButton->addElement(this->backIcon);
        this->backText = new Aether::Text(this->backIcon->x() + this->backIcon->w() + 20, 32, "Common.Back"_lang, 26);
        this->backText->setY(this->backText->y() - this->backText->h()/2);
        this->backText->setColour(this->app->theme()->FG());
        this->backButton->addElement(this->backText);
        this->backButton->setCallback([this]() {
            this->backCallback();
        });
        this->backButton->setSelectable(false);
        this->touchContainer->addElement(this->backButton);

        // Quit
        Aether::Element * quitButton = new Aether::Element(155, 0, 155, 65);
        Aether::Text * quitText = new Aether::Text(quitButton->x() + 30, 32, "Common.Quit"_lang, 26);
        quitText->setY(quitText->y() - quitText->h()/2);
        quitText->setColour(this->app->theme()->FG());
        quitButton->addElement(quitText);
        Aether::Image * quitIcon = new Aether::Image(quitText->x() + quitText->w() + 20, 22, "romfs:/icons/quit.png");
        quitIcon->setColour(this->app->theme()->FG());
        quitButton->addElement(quitIcon);
        quitButton->setCallback([this]() {
            this->app->exit(false);
        });
        quitButton->setSelectable(false);
        this->touchContainer->addElement(quitButton);
        this->sideContainer->addElement(this->touchContainer);

        // Navigation list
        this->sideSearch = new CustomElm::SideButton(10, 80, 290);
        this->sideSearch->setIcon(new Aether::Image(0, 0, "romfs:/icons/search.png"));
        this->sideSearch->setText("Search.Search"_lang);
        this->sideSearch->setActiveColour(this->app->theme()->accent());
        this->sideSearch->setInactiveColour(this->app->theme()->FG());
        this->sideSearch->setCallback([this](){
            // If the current frame is search recreate it
            if (this->frameType == Frame::Type::Search) {
                this->returnElement(this->playerDim);
                this->container->removeElement(this->frame);
                this->frame = new Frame::Search(this->app);
                this->finalizeState();

            // Otherwise push the last frame on the stack
            } else {
                this->changeFrame(Frame::Type::Search, Frame::Action::Push);
            }
        });
        this->sideContainer->addElement(this->sideSearch);
        this->sideSeparator = new Aether::Rectangle(30, this->sideSearch->y() + 70, 250, 1);
        this->sideSeparator->setColour(this->app->theme()->muted2());
        this->sideContainer->addElement(this->sideSeparator);
        this->sidePlaylists = new CustomElm::SideButton(10, this->sideSeparator->y() + 10, 290);
        this->sidePlaylists->setIcon(new Aether::Image(0, 0, "romfs:/icons/playlist.png"));
        this->sidePlaylists->setText("Playlist.Playlists"_lang);
        this->sidePlaylists->setActiveColour(this->app->theme()->accent());
        this->sidePlaylists->setInactiveColour(this->app->theme()->FG());
        this->sidePlaylists->setCallback([this](){
            this->changeFrame(Frame::Type::Playlists, Frame::Action::Reset);
        });
        this->sideContainer->addElement(this->sidePlaylists);
        this->sideSongs = new CustomElm::SideButton(10, this->sidePlaylists->y() + 60, 290);
        this->sideSongs->setIcon(new Aether::Image(0, 0, "romfs:/icons/musicnote.png"));
        this->sideSongs->setText("Song.Songs"_lang);
        this->sideSongs->setActiveColour(this->app->theme()->accent());
        this->sideSongs->setInactiveColour(this->app->theme()->FG());
        this->sideSongs->setCallback([this](){
            this->changeFrame(Frame::Type::Songs, Frame::Action::Reset);
        });
        this->sideContainer->addElement(this->sideSongs);
        this->sideArtists = new CustomElm::SideButton(10, this->sideSongs->y() + 60, 290);
        this->sideArtists->setIcon(new Aether::Image(0, 0, "romfs:/icons/user.png"));
        this->sideArtists->setText("Artist.Artists"_lang);
        this->sideArtists->setActiveColour(this->app->theme()->accent());
        this->sideArtists->setInactiveColour(this->app->theme()->FG());
        this->sideArtists->setCallback([this](){
            this->changeFrame(Frame::Type::Artists, Frame::Action::Reset);
        });
        this->sideContainer->addElement(this->sideArtists);
        this->sideAlbums = new CustomElm::SideButton(10, this->sideArtists->y() + 60, 290);
        this->sideAlbums->setIcon(new Aether::Image(0, 0, "romfs:/icons/disc.png"));
        this->sideAlbums->setText("Album.Albums"_lang);
        this->sideAlbums->setActiveColour(this->app->theme()->accent());
        this->sideAlbums->setInactiveColour(this->app->theme()->FG());
        this->sideAlbums->setCallback([this](){
            this->changeFrame(Frame::Type::Albums, Frame::Action::Reset);
        });
        this->sideContainer->addElement(this->sideAlbums);
        this->sideSeparator2 = new Aether::Rectangle(30, this->sideAlbums->y() + 70, 250, 1);
        this->sideSeparator2->setColour(this->app->theme()->muted2());
        this->sideContainer->addElement(this->sideSeparator2);
        this->sideQueue = new CustomElm::SideButton(10, this->sideSeparator2->y() + 10, 290);
        this->sideQueue->setIcon(new Aether::Image(0, 0, "romfs:/icons/queue.png"));
        this->sideQueue->setText("Queue.Heading"_lang);
        this->sideQueue->setActiveColour(this->app->theme()->accent());
        this->sideQueue->setInactiveColour(this->app->theme()->FG());
        this->sideQueue->setCallback([this](){
            // If the current frame is queue recreate it
            if (this->frameType == Frame::Type::Queue) {
                this->returnElement(this->playerDim);
                this->container->removeElement(this->frame);
                this->frame = new Frame::Queue(this->app);
                this->finalizeState();

            // Otherwise push the last frame on the stack
            } else {
                this->changeFrame(Frame::Type::Queue, Frame::Action::Push);
            }
        });
        this->sideContainer->addElement(this->sideQueue);
        this->sideSeparator3 = new Aether::Rectangle(30, this->sideQueue->y() + 70, 250, 1);
        this->sideSeparator3->setColour(this->app->theme()->muted2());
        this->sideContainer->addElement(this->sideSeparator3);
        this->sideSettings = new CustomElm::SideButton(10, this->sideSeparator3->y() + 10, 290);
        this->sideSettings->setIcon(new Aether::Image(0, 0, "romfs:/icons/settings.png"));
        this->sideSettings->setText("Settings.Settings"_lang);
        this->sideSettings->setActiveColour(this->app->theme()->accent());
        this->sideSettings->setInactiveColour(this->app->theme()->FG());
        this->sideSettings->setCallback([this](){
            this->app->pushScreen();
            this->app->setScreen(Main::ScreenID::Settings);
        });
        this->sideContainer->addElement(this->sideSettings);

        this->updateDot = new Aether::Ellipse(this->sideSettings->x() + this->sideSettings->w() - this->sideSettings->h()/2 - 8, this->sideSettings->y() + this->sideSettings->h()/2 - 8, 16);
        this->updateDot->setColour(this->app->theme()->accent());
        this->sideContainer->addElement(this->updateDot);

        // Set appropriate button active
        switch (this->app->config()->initialFrame()) {
            case Frame::Type::Playlists:
                this->sideContainer->setFocussed(this->sidePlaylists);
                break;

            case Frame::Type::Songs:
                this->sideContainer->setFocussed(this->sideSongs);
                break;

            case Frame::Type::Artists:
                this->sideContainer->setFocussed(this->sideArtists);
                break;

            case Frame::Type::Albums:
                this->sideContainer->setFocussed(this->sideAlbums);
                break;

            default:
                break;
        }
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
            this->app->pushScreen();
            this->app->setScreen(Main::ScreenID::Fullscreen);
        });
        this->addElement(this->player);
        this->player->setHasSelectable(false);

        // Set songs active
        this->frame = nullptr;
        this->frameType = Frame::Type::None;
        this->changeFrame(this->app->config()->initialFrame(), Frame::Action::Reset);
        this->container->setFocussed(this->frame);

        // Init vars
        this->addToPlMenu = nullptr;
        this->backOneFrame = 0;
        this->confirmQueue = nullptr;
        this->playingID = -100;     // This number needs to be less than -1, as >= -1 are valid values
    }

    void Home::onUnload() {
        Screen::onUnload();

        delete this->addToPlMenu;
        delete this->confirmQueue;

        // Ensure all frames are deleted
        this->container->removeElement(this->frame);
        while (!this->frameStack.empty()) {
            delete this->frameStack.top().frame;
            this->frameStack.pop();
        }

        // The rest of this isn't necessary in this context but it's good to do so
        this->removeElement(this->bg);
        this->removeElement(this->sidegrad);
        this->removeElement(this->sideBg);
        this->removeElement(this->playerDim);
        this->removeElement(this->player);
        this->removeElement(this->container);
    }
};