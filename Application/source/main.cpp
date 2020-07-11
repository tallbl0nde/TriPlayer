#include "Application.hpp"
#include "FS.hpp"
#include "Log.hpp"
#include "MP3.hpp"

// Main folder
#define MAIN_FOLDER "/switch/TriPlayer/"
// Log name
#define LOG_FILE "application.log"

int main(void) {
    // Ensure directory exists
    Utils::Fs::createPath(MAIN_FOLDER);

    // Start logging
    Log::openFile(MAIN_FOLDER LOG_FILE, Log::Level::Success);
    Log::writeSuccess("=== Application Launched ===");

    // Start services
    romfsInit();
    socketInitializeDefault();
    Utils::MP3::init();

    // Start actual 'app' execution
    Main::Application * app = new Main::Application();
    app->run();
    delete app;

    // Stop services
    Utils::MP3::exit();
    socketExit();
    romfsExit();

    return 0;
}