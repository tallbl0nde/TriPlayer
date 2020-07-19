#include "Application.hpp"
#include "Log.hpp"
#include "utils/Curl.hpp"
#include "utils/FS.hpp"
#include "utils/MP3.hpp"

// Main folder
#define MAIN_FOLDER "/switch/TriPlayer/"
#define ARTIST_IMAGES MAIN_FOLDER "images/artist"
// Log name
#define LOG_FILE "application.log"

int main(void) {
    // Ensure directories exist
    Utils::Fs::createPath(MAIN_FOLDER);
    Utils::Fs::createPath(ARTIST_IMAGES);

    // Start logging
    Log::openFile(MAIN_FOLDER LOG_FILE, Log::Level::Success);
    Log::writeSuccess("=== Application Launched ===");

    // Start services
    romfsInit();
    socketInitializeDefault();
    Utils::Curl::init();
    Utils::MP3::init();

    // Start actual 'app' execution
    Main::Application * app = new Main::Application();
    app->run();
    delete app;

    // Stop services
    Utils::MP3::exit();
    Utils::Curl::exit();
    socketExit();
    romfsExit();

    return 0;
}