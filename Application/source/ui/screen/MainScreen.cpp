#include "MainScreen.hpp"

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

    void MainScreen::onLoad() {
        // Create side menu
        this->menuRP = new CustomElm::SideButton(20, 150, 310);
        this->menuRP->setIcon(new Aether::Image(0, 0, "romfs:/icons/clock.png"));
        this->menuRP->setText("Recently Played");
        this->menuRP->setActiveColour(this->app->theme()->accent());
        this->menuRP->setInactiveColour(this->app->theme()->text());
        this->menuRP->setCallback([this](){
            this->deselectSideItems();
            this->menuRP->setActivated(true);

        });
        this->addElement(this->menuRP);
        this->menuSongs = new CustomElm::SideButton(20, 210, 310);
        this->menuSongs->setIcon(new Aether::Image(0, 0, "romfs:/icons/musicnote.png"));
        this->menuSongs->setText("Songs");
        this->menuSongs->setActiveColour(this->app->theme()->accent());
        this->menuSongs->setInactiveColour(this->app->theme()->text());
        this->menuSongs->setCallback([this](){
            this->deselectSideItems();
            this->menuSongs->setActivated(true);
        });
        this->addElement(this->menuSongs);
        this->menuArtists = new CustomElm::SideButton(20, 270, 310);
        this->menuArtists->setIcon(new Aether::Image(0, 0, "romfs:/icons/user.png"));
        this->menuArtists->setText("Artists");
        this->menuArtists->setActiveColour(this->app->theme()->accent());
        this->menuArtists->setInactiveColour(this->app->theme()->text());
        this->menuArtists->setCallback([this](){
            this->deselectSideItems();
            this->menuArtists->setActivated(true);
        });
        this->addElement(this->menuArtists);
        this->menuAlbums = new CustomElm::SideButton(20, 330, 310);
        this->menuAlbums->setIcon(new Aether::Image(0, 0, "romfs:/icons/disc.png"));
        this->menuAlbums->setText("Albums");
        this->menuAlbums->setActiveColour(this->app->theme()->accent());
        this->menuAlbums->setInactiveColour(this->app->theme()->text());
        this->menuAlbums->setCallback([this](){
            this->deselectSideItems();
            this->menuAlbums->setActivated(true);
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
            this->deselectSideItems();
            this->menuQueue->setActivated(true);
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
            this->deselectSideItems();
            this->menuSettings->setActivated(true);
        });
        this->addElement(this->menuSettings);

        this->setFocussed(this->menuSongs);
        this->menuSongs->setActivated(true);
    }

    void MainScreen::onUnload() {
        this->removeElement(this->menuRP);
        this->removeElement(this->menuSongs);
        this->removeElement(this->menuArtists);
        this->removeElement(this->menuAlbums);
        this->removeElement(this->menuQueue);
        this->removeElement(this->menuSettings);
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