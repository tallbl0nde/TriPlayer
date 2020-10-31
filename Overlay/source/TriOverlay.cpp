#include "Database.hpp"
#include "ipc/TriPlayer.hpp"
#include "gui/Error.hpp"
#include "gui/Player.hpp"
#include "TriOverlay.hpp"

TriOverlay::TriOverlay() : tsl::Overlay() {
    this->triInitialized = false;
}

void TriOverlay::initServices() {
    // Mount sd card
    fsdevMountSdmc();

    // Attempt to connect to TriPlayer
    this->triInitialized = TriPlayer::initialize();

    // Attempt to open connection to database
    this->database = new Database();
    this->dbInitialized = this->database->openReadOnly();
}

void TriOverlay::exitServices() {
    delete this->database;

    if (this->triInitialized) {
        TriPlayer::exit();
        this->triInitialized = false;
    }

    fsdevUnmountDevice("sdmc");
}

std::unique_ptr<tsl::Gui> TriOverlay::loadInitialGui() {
    // Show error frame if service failed to initialize
    if (!this->dbInitialized || !this->triInitialized) {
        return std::make_unique<Gui::Error>();
    }

    // Otherwise proceed to normal (player) frame
    return std::make_unique<Gui::Player>(this->database);
}