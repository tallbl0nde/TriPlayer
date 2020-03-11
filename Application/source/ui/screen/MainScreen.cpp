#include "ListSong.hpp"
#include "MainScreen.hpp"
#include "Types.hpp"

namespace Screen {
    MainScreen::MainScreen(Main::Application * a) : Screen() {
        this->app = a;

        this->onButtonPress(Aether::Button::B, [this](){
            this->app->exit();
        });
    }

    void MainScreen::deselectSideItems() {
        this->sideRP->setActivated(false);
        this->sideSongs->setActivated(false);
        this->sideArtists->setActivated(false);
        this->sideAlbums->setActivated(false);
        this->sideQueue->setActivated(false);
    }

    void MainScreen::setupSongs() {
        this->deselectSideItems();
        this->sideSongs->setActivated(true);
        // this->heading->setString("Songs");
    }

    void MainScreen::onLoad() {
        // Add background images
        this->bg = new Aether::Image(0, 0, "romfs:/bg/main.png");
        this->addElement(this->bg);
        this->sidegrad = new Aether::Image(310, 15, "romfs:/misc/sidegradient.png");
        this->sidegrad->setColour(Aether::Colour{255, 255, 255, 200});
        this->addElement(this->sidegrad);

        // === SIDEBAR ===
        this->sideBg = new Aether::Rectangle(0, 0, 310, 590);
        this->sideBg->setColour(this->app->theme()->sideBG());
        this->addElement(this->sideBg);
        // User
        this->userBg = new Aether::Rectangle(25, 30, 200, 50, 10);
        this->userBg->setColour(this->app->theme()->muted2());
        this->addElement(this->userBg);
        this->userIcon = new Aether::Image(this->userBg->x(), this->userBg->y(), "romfs:/user.png");
        this->userIcon->setWH(50, 50);
        this->addElement(this->userIcon);
        this->userText = new Aether::Text(this->userIcon->x() + this->userIcon->w() + 8, 0, "Username", 26);
        this->userText->setY(this->userBg->y() + (this->userBg->h() - this->userText->h())/2);
        this->userText->setColour(this->app->theme()->FG());
        this->addElement(this->userText);

        // Settings
        this->settingsBg = new Aether::Rectangle(235, 30, 50, 50, 10);
        this->settingsBg->setColour(this->app->theme()->muted2());
        this->addElement(this->settingsBg);
        this->settingsIcon = new Aether::Image(this->settingsBg->x() + 10, this->settingsBg->y() + 10, "romfs:/icons/settings.png");
        this->addElement(this->settingsIcon);

        // Search bar
        this->search = new CustomElm::SearchBox(25, 130, 260);
        this->search->setBoxColour(this->app->theme()->muted2());
        this->search->setIconColour(this->app->theme()->FG());
        this->addElement(this->search);

        // Navigation list
        this->sideRP = new CustomElm::SideButton(10, 210, 290);
        this->sideRP->setIcon(new Aether::Image(0, 0, "romfs:/icons/clock.png"));
        this->sideRP->setText("Recently Played");
        this->sideRP->setActiveColour(this->app->theme()->accent());
        this->sideRP->setInactiveColour(this->app->theme()->FG());
        this->sideRP->setCallback([this](){

        });
        this->addElement(this->sideRP);
        this->sideSongs = new CustomElm::SideButton(10, this->sideRP->y() + 60, 290);
        this->sideSongs->setIcon(new Aether::Image(0, 0, "romfs:/icons/musicnote.png"));
        this->sideSongs->setText("Songs");
        this->sideSongs->setActiveColour(this->app->theme()->accent());
        this->sideSongs->setInactiveColour(this->app->theme()->FG());
        this->sideSongs->setCallback([this](){
            this->setupSongs();
        });
        this->addElement(this->sideSongs);
        this->sideArtists = new CustomElm::SideButton(10, this->sideSongs->y() + 60, 290);
        this->sideArtists->setIcon(new Aether::Image(0, 0, "romfs:/icons/user.png"));
        this->sideArtists->setText("Artists");
        this->sideArtists->setActiveColour(this->app->theme()->accent());
        this->sideArtists->setInactiveColour(this->app->theme()->FG());
        this->sideArtists->setCallback([this](){

        });
        this->addElement(this->sideArtists);
        this->sideAlbums = new CustomElm::SideButton(10, this->sideArtists->y() + 60, 290);
        this->sideAlbums->setIcon(new Aether::Image(0, 0, "romfs:/icons/disc.png"));
        this->sideAlbums->setText("Albums");
        this->sideAlbums->setActiveColour(this->app->theme()->accent());
        this->sideAlbums->setInactiveColour(this->app->theme()->FG());
        this->sideAlbums->setCallback([this](){

        });
        this->addElement(this->sideAlbums);
        this->sideSeparator = new Aether::Rectangle(30, this->sideAlbums->y() + 70, 250, 1);
        this->sideSeparator->setColour(this->app->theme()->muted2());
        this->addElement(this->sideSeparator);
        this->sideQueue = new CustomElm::SideButton(10, this->sideSeparator->y() + 10, 290);
        this->sideQueue->setIcon(new Aether::Image(0, 0, "romfs:/icons/queue.png"));
        this->sideQueue->setText("Queue");
        this->sideQueue->setActiveColour(this->app->theme()->accent());
        this->sideQueue->setInactiveColour(this->app->theme()->FG());
        this->sideQueue->setCallback([this](){

        });
        this->addElement(this->sideQueue);

        // === PLAYER ===
        this->playerBg = new Aether::Rectangle(0, 590, 1280, 130);
        this->playerBg->setColour(this->app->theme()->bottomBG());
        this->addElement(this->playerBg);
        // Album/song playing
        this->albumCover = new Aether::Image(10, 600, "romfs:/misc/noalbum.png");
        this->albumCover->setWH(110, 110);
        this->addElement(this->albumCover);
        this->trackName = new Aether::Text(140, 625, "Nothing playing!", 24);
        this->trackName->setColour(this->app->theme()->FG());
        this->addElement(this->trackName);
        this->trackArtist = new Aether::Text(140, 660, "Play a song", 18);
        this->trackArtist->setColour(this->app->theme()->muted());
        this->addElement(this->trackArtist);

        // Controls
        this->shuffle = new Aether::Image(480, 630, "romfs:/icons/shuffle.png");
        this->shuffle->setColour(this->app->theme()->muted());
        this->shuffle->setCallback([this]() {
            // Shuffle toggle here
        });
        this->addElement(this->shuffle);
        this->previous = new Aether::Image(550, 628, "romfs:/icons/previous.png");
        this->previous->setColour(this->app->theme()->FG());
        this->previous->setCallback([this]() {
            this->app->sysmodule()->previousSong();
        });
        this->addElement(this->previous);
        this->play = new Aether::Image(610, 610, "romfs:/icons/play.png");
        this->play->setColour(this->app->theme()->FG());
        this->play->setCallback([this]() {
            this->app->sysmodule()->resumePlayback();
            this->pause->setHidden(false);
            this->play->setHidden(true);
            this->setFocussed(this->pause);
        });
        this->play->setHidden(true);
        this->addElement(this->play);
        this->pause = new Aether::Image(610, 610, "romfs:/icons/pause.png");
        this->pause->setColour(this->app->theme()->FG());
        this->pause->setCallback([this]() {
            this->app->sysmodule()->pausePlayback();
            this->pause->setHidden(true);
            this->play->setHidden(false);
            this->setFocussed(this->play);
        });
        this->addElement(this->pause);
        this->next = new Aether::Image(705, 628, "romfs:/icons/next.png");
        this->next->setColour(this->app->theme()->FG());
        this->next->setCallback([this]() {
            this->app->sysmodule()->nextSong();
        });
        this->addElement(this->next);
        this->repeat = new Aether::Image(770, 630, "romfs:/icons/repeat.png");
        this->repeat->setColour(this->app->theme()->muted());
        this->repeat->setCallback([this]() {
            // Repeat toggle here
        });
        this->addElement(this->repeat);

        // Seeking
        this->position = new Aether::Text(0, 0, "0:00", 18);
        this->position->setX(420 - this->position->w());
        this->position->setY(693 - this->position->h()/2);
        this->position->setColour(this->app->theme()->FG());
        this->addElement(this->position);
        this->seekBar = new Aether::RoundProgressBar(440, 690, 400, 8);
        this->seekBar->setBackgroundColour(this->app->theme()->muted2());
        this->seekBar->setForegroundColour(this->app->theme()->accent());
        this->addElement(this->seekBar);
        this->duration = new Aether::Text(860, 0, "0:00", 18);
        this->duration->setY(693 - this->duration->h()/2);
        this->duration->setColour(this->app->theme()->FG());
        this->addElement(this->duration);

        // Volume + full screen
        this->volumeIcon = new Aether::Image(970, 638, "romfs:/icons/volume.png");
        this->volumeIcon->setColour(this->app->theme()->FG());
        this->volumeIcon->setCallback([this]() {
            // Toggle volume here
        });
        this->addElement(this->volumeIcon);
        this->volume = new Aether::RoundProgressBar(1025, 650, 130, 8);
        this->volume->setBackgroundColour(this->app->theme()->muted2());
        this->volume->setForegroundColour(this->app->theme()->accent());
        this->volume->setValue(100.0);
        this->addElement(this->volume);
        this->fullscreen = new Aether::Image(1195, 638, "romfs:/icons/fullscreen.png");
        this->fullscreen->setColour(this->app->theme()->muted());
        this->fullscreen->setCallback([this]() {
            // Change screen
        });
        this->addElement(this->fullscreen);

        // === MAIN ===
        // Heading
        this->heading = new Aether::Text(420, 40, "", 60);
        this->heading->setColour(this->app->theme()->FG());
        this->addElement(this->heading);

        // Add list headings
        this->titleH = new Aether::Text(415, 160, "Title", 20);
        this->titleH->setColour(this->app->theme()->muted());
        this->addElement(this->titleH);
        this->artistH = new Aether::Text(773, 160, "Artist", 20);
        this->artistH->setColour(this->app->theme()->muted());
        this->addElement(this->artistH);
        this->albumH = new Aether::Text(965, 160, "Album", 20);
        this->albumH->setColour(this->app->theme()->muted());
        this->addElement(this->albumH);
        this->lengthH = new Aether::Text(1215, 160, "Length", 20);
        this->lengthH->setX(this->lengthH->x() - this->lengthH->w());
        this->lengthH->setColour(this->app->theme()->muted());
        this->addElement(this->lengthH);

        // Create list
        this->list = new Aether::List(320, 190, 950, 400);
        this->list->setScrollBarColour(this->app->theme()->muted2());
        this->list->setShowScrollBar(true);
        this->addElement(this->list);

        // Create items for each songs
        std::vector<SongInfo> si = this->app->database()->getAllSongInfo();
        for (size_t i = 0; i < si.size(); i++) {
            CustomElm::ListSong * l = new CustomElm::ListSong();
            l->setTitleString(si[i].title);
            l->setArtistString(si[i].artist);
            l->setAlbumString(si[i].album);
            l->setLengthString(std::to_string(si[i].duration/60) + ":" + std::to_string(si[i].duration%60));
            l->setLineColour(this->app->theme()->muted2());
            l->setTextColour(this->app->theme()->FG());
            SongID id = si[i].ID;
            l->setCallback([this, id](){
                this->app->sysmodule()->playSong(id);
            });
            this->list->addElement(l);

            if (i == 0) {
                l->setY(this->list->y() + 20);
            }
        }

        // Set songs active
        this->setFocussed(this->sideSongs);
        this->setupSongs();
    }

    void MainScreen::onUnload() {
        this->removeElement(this->bg);
        this->removeElement(this->sidegrad);
        this->removeElement(this->sideBg);
        this->removeElement(this->userBg);
        this->removeElement(this->userIcon);
        this->removeElement(this->userText);
        this->removeElement(this->settingsBg);
        this->removeElement(this->settingsIcon);
        this->removeElement(this->search);
        this->removeElement(this->sideRP);
        this->removeElement(this->sideSongs);
        this->removeElement(this->sideArtists);
        this->removeElement(this->sideAlbums);
        this->removeElement(this->sideSeparator);
        this->removeElement(this->sideQueue);
        this->removeElement(this->playerBg);
        this->removeElement(this->albumCover);
        this->removeElement(this->trackName);
        this->removeElement(this->trackArtist);
        this->removeElement(this->shuffle);
        this->removeElement(this->previous);
        this->removeElement(this->play);
        this->removeElement(this->pause);
        this->removeElement(this->next);
        this->removeElement(this->repeat);
        this->removeElement(this->position);
        this->removeElement(this->seekBar);
        this->removeElement(this->duration);
        this->removeElement(this->volumeIcon);
        this->removeElement(this->volume);
        this->removeElement(this->fullscreen);
        // this->removeElement(this->heading);
        // this->removeElement(this->list);
    }
};