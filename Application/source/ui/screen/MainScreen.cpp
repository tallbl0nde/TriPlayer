#include "MainScreen.hpp"

namespace Screen {
    MainScreen::MainScreen(Main::Application * a) : Screen() {
        this->app = a;

        this->onButtonPress(Aether::Button::B, [this](){
            this->app->exit();
        });
    }

    void MainScreen::onLoad() {
        // Create side menu
        this->menuRP = new CustomElm::SideButton(20, 150, 310);
        this->menuRP->setText("Recently Played");
        this->menuRP->setActiveColour(Aether::Theme::Dark.accent);
        this->menuRP->setInactiveColour(Aether::Theme::Dark.text);
        this->menuRP->setCallback([](){

        });
        this->addElement(this->menuRP);
        this->menuSongs = new CustomElm::SideButton(20, 210, 310);
        this->menuSongs->setText("Songs");
        this->menuSongs->setActiveColour(Aether::Theme::Dark.accent);
        this->menuSongs->setInactiveColour(Aether::Theme::Dark.text);
        this->menuSongs->setCallback([](){
        });
        this->menuSongs->setActivated(true);
        this->addElement(this->menuSongs);
        this->menuArtists = new CustomElm::SideButton(20, 270, 310);
        this->menuArtists->setText("Artists");
        this->menuArtists->setActiveColour(Aether::Theme::Dark.accent);
        this->menuArtists->setInactiveColour(Aether::Theme::Dark.text);
        this->menuArtists->setCallback([](){
        });
        this->addElement(this->menuArtists);
        this->menuAlbums = new CustomElm::SideButton(20, 330, 310);
        this->menuAlbums->setText("Albums");
        this->menuAlbums->setActiveColour(Aether::Theme::Dark.accent);
        this->menuAlbums->setInactiveColour(Aether::Theme::Dark.text);
        this->menuAlbums->setCallback([](){
        });
        this->addElement(this->menuAlbums);
        this->menuQueue = new CustomElm::SideButton(20, 390, 310);
        this->menuQueue->setText("Queue");
        this->menuQueue->setActiveColour(Aether::Theme::Dark.accent);
        this->menuQueue->setInactiveColour(Aether::Theme::Dark.text);
        this->menuQueue->setCallback([](){
        });
        this->addElement(this->menuQueue);
        this->menuSettings = new CustomElm::SideButton(20, 450, 310);
        this->menuSettings->setText("Settings");
        this->menuSettings->setActiveColour(Aether::Theme::Dark.accent);
        this->menuSettings->setInactiveColour(Aether::Theme::Dark.text);
        this->menuSettings->setCallback([](){
        });
        this->addElement(this->menuSettings);
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