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
        this->menuRP->setActivated(false);
        this->menuSongs->setActivated(false);
        this->menuArtists->setActivated(false);
        this->menuAlbums->setActivated(false);
        this->menuQueue->setActivated(false);
        this->menuSettings->setActivated(false);
    }

    void MainScreen::setupSongs() {
        this->deselectSideItems();
        this->menuSongs->setActivated(true);
        this->heading->setString("Songs");
    }

    void MainScreen::onLoad() {
        // Create side menu
        this->menuRP = new CustomElm::SideButton(20, 150, 310);
        this->menuRP->setIcon(new Aether::Image(0, 0, "romfs:/icons/clock.png"));
        this->menuRP->setText("Recently Played");
        this->menuRP->setActiveColour(this->app->theme()->accent());
        this->menuRP->setInactiveColour(this->app->theme()->text());
        this->menuRP->setCallback([this](){

        });
        this->addElement(this->menuRP);
        this->menuSongs = new CustomElm::SideButton(20, 210, 310);
        this->menuSongs->setIcon(new Aether::Image(0, 0, "romfs:/icons/musicnote.png"));
        this->menuSongs->setText("Songs");
        this->menuSongs->setActiveColour(this->app->theme()->accent());
        this->menuSongs->setInactiveColour(this->app->theme()->text());
        this->menuSongs->setCallback([this](){
            this->setupSongs();
        });
        this->addElement(this->menuSongs);
        this->menuArtists = new CustomElm::SideButton(20, 270, 310);
        this->menuArtists->setIcon(new Aether::Image(0, 0, "romfs:/icons/user.png"));
        this->menuArtists->setText("Artists");
        this->menuArtists->setActiveColour(this->app->theme()->accent());
        this->menuArtists->setInactiveColour(this->app->theme()->text());
        this->menuArtists->setCallback([this](){

        });
        this->addElement(this->menuArtists);
        this->menuAlbums = new CustomElm::SideButton(20, 330, 310);
        this->menuAlbums->setIcon(new Aether::Image(0, 0, "romfs:/icons/disc.png"));
        this->menuAlbums->setText("Albums");
        this->menuAlbums->setActiveColour(this->app->theme()->accent());
        this->menuAlbums->setInactiveColour(this->app->theme()->text());
        this->menuAlbums->setCallback([this](){

        });
        this->addElement(this->menuAlbums);

        Aether::Rectangle * r = new Aether::Rectangle(40, 400, 270, 1);
        r->setColour(this->app->theme()->mutedLine());
        this->addElement(r);

        this->menuQueue = new CustomElm::SideButton(20, 410, 310);
        this->menuQueue->setIcon(new Aether::Image(0, 0, "romfs:/icons/queue.png"));
        this->menuQueue->setText("Queue");
        this->menuQueue->setActiveColour(this->app->theme()->accent());
        this->menuQueue->setInactiveColour(this->app->theme()->text());
        this->menuQueue->setCallback([this](){

        });
        this->addElement(this->menuQueue);

        r = new Aether::Rectangle(40, 480, 270, 1);
        r->setColour(this->app->theme()->mutedLine());
        this->addElement(r);

        this->menuSettings = new CustomElm::SideButton(20, 490, 310);
        this->menuSettings->setIcon(new Aether::Image(0, 0, "romfs:/icons/settings.png"));
        this->menuSettings->setText("Settings");
        this->menuSettings->setActiveColour(this->app->theme()->accent());
        this->menuSettings->setInactiveColour(this->app->theme()->text());
        this->menuSettings->setCallback([this](){

        });
        this->addElement(this->menuSettings);

        // Add heading and search box
        this->heading = new Aether::Text(420, 40, "", 60);
        this->heading->setColour(this->app->theme()->heading());
        this->addElement(this->heading);

        this->search = new CustomElm::SearchBox(910, 60, 300, 40);
        this->search->setColour(this->app->theme()->heading());
        this->addElement(this->search);

        // Set songs active
        this->setFocussed(this->menuSongs);
        this->setupSongs();

        // Add list headings
        this->titleH = new Aether::Text(415, 160, "Title", 20);
        this->titleH->setColour(this->app->theme()->mutedText());
        this->addElement(this->titleH);
        this->artistH = new Aether::Text(773, 160, "Artist", 20);
        this->artistH->setColour(this->app->theme()->mutedText());
        this->addElement(this->artistH);
        this->albumH = new Aether::Text(965, 160, "Album", 20);
        this->albumH->setColour(this->app->theme()->mutedText());
        this->addElement(this->albumH);
        this->lengthH = new Aether::Text(1215, 160, "Length", 20);
        this->lengthH->setX(this->lengthH->x() - this->lengthH->w());
        this->lengthH->setColour(this->app->theme()->mutedText());
        this->addElement(this->lengthH);

        // Create list
        this->list = new Aether::List(350, 190, 930, 400);
        this->list->setScrollBarColour(this->app->theme()->mutedLine());
        this->list->setShowScrollBar(true);
        this->addElement(this->list);

        // Create items for each songs
        std::vector<SongInfo> si = this->app->database()->getAllSongInfo();
        for (size_t i = 0; i < si.size(); i++) {
            CustomElm::ListSong * l = new CustomElm::ListSong();
            l->setTitleString(si[i].title);
            l->setArtistString(si[i].artist);
            l->setAlbumString(si[i].album);
            l->setLengthString("0:00");
            l->setLineColour(this->app->theme()->mutedLine());
            l->setTextColour(this->app->theme()->text());
            l->setCallback([](){
                // something using si[i].id here
            });
            this->list->addElement(l);

            if (i == 0) {
                l->setY(this->list->y() + 20);
            }
        }
    }

    void MainScreen::onUnload() {
        this->removeElement(this->menuRP);
        this->removeElement(this->menuSongs);
        this->removeElement(this->menuArtists);
        this->removeElement(this->menuAlbums);
        this->removeElement(this->menuQueue);
        this->removeElement(this->menuSettings);
        this->removeElement(this->heading);
        this->removeElement(this->search);
        this->removeElement(this->list);
    }

    void MainScreen::render() {
        // Left pane
        SDLHelper::drawFilledRect(this->app->theme()->sideBG(), 0, 0, 350, 590);
        // Bottom pane
        SDLHelper::drawFilledRect(this->app->theme()->bottomBG(), 0, 590, 1280, 130);

        // Render as per usual
        Screen::render();
    }
};