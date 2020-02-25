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
        this->menuRP->setActiveColour(Aether::Theme::Dark.accent);
        this->menuRP->setInactiveColour(Aether::Theme::Dark.text);
        this->menuRP->setCallback([this](){
            this->deselectSideItems();
            this->menuRP->setActivated(true);

        });
        this->addElement(this->menuRP);
        this->menuSongs = new CustomElm::SideButton(20, 210, 310);
        this->menuSongs->setIcon(new Aether::Image(0, 0, "romfs:/icons/musicnote.png"));
        this->menuSongs->setText("Songs");
        this->menuSongs->setActiveColour(Aether::Theme::Dark.accent);
        this->menuSongs->setInactiveColour(Aether::Theme::Dark.text);
        this->menuSongs->setCallback([this](){
            this->deselectSideItems();
            this->menuSongs->setActivated(true);
        });
        this->addElement(this->menuSongs);
        this->menuArtists = new CustomElm::SideButton(20, 270, 310);
        this->menuArtists->setIcon(new Aether::Image(0, 0, "romfs:/icons/user.png"));
        this->menuArtists->setText("Artists");
        this->menuArtists->setActiveColour(Aether::Theme::Dark.accent);
        this->menuArtists->setInactiveColour(Aether::Theme::Dark.text);
        this->menuArtists->setCallback([this](){
            this->deselectSideItems();
            this->menuArtists->setActivated(true);
        });
        this->addElement(this->menuArtists);
        this->menuAlbums = new CustomElm::SideButton(20, 330, 310);
        this->menuAlbums->setIcon(new Aether::Image(0, 0, "romfs:/icons/disc.png"));
        this->menuAlbums->setText("Albums");
        this->menuAlbums->setActiveColour(Aether::Theme::Dark.accent);
        this->menuAlbums->setInactiveColour(Aether::Theme::Dark.text);
        this->menuAlbums->setCallback([this](){
            this->deselectSideItems();
            this->menuAlbums->setActivated(true);
        });
        this->addElement(this->menuAlbums);

        Aether::Rectangle * r = new Aether::Rectangle(40, 400, 270, 1);
        r->setColour(Aether::Colour{60, 60, 100, 255});
        this->addElement(r);

        this->menuQueue = new CustomElm::SideButton(20, 410, 310);
        this->menuQueue->setIcon(new Aether::Image(0, 0, "romfs:/icons/queue.png"));
        this->menuQueue->setText("Queue");
        this->menuQueue->setActiveColour(Aether::Theme::Dark.accent);
        this->menuQueue->setInactiveColour(Aether::Theme::Dark.text);
        this->menuQueue->setCallback([this](){
            this->deselectSideItems();
            this->menuQueue->setActivated(true);
        });
        this->addElement(this->menuQueue);

        r = new Aether::Rectangle(40, 480, 270, 1);
        r->setColour(Aether::Colour{60, 60, 100, 255});
        this->addElement(r);

        this->menuSettings = new CustomElm::SideButton(20, 490, 310);
        this->menuSettings->setIcon(new Aether::Image(0, 0, "romfs:/icons/settings.png"));
        this->menuSettings->setText("Settings");
        this->menuSettings->setActiveColour(Aether::Theme::Dark.accent);
        this->menuSettings->setInactiveColour(Aether::Theme::Dark.text);
        this->menuSettings->setCallback([this](){
            this->deselectSideItems();
            this->menuSettings->setActivated(true);
        });
        this->addElement(this->menuSettings);
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
        SDLHelper::drawFilledRect(Aether::Colour{30, 30, 45, 255}, 0, 0, 350, 590);
        // Bottom pane
        SDLHelper::drawFilledRect(Aether::Colour{40, 40, 50, 255}, 0, 590, 1280, 130);

        // Render as per usual
        Screen::render();
    }
};