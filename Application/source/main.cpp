#include "Application.hpp"
#include "Log.hpp"
#include "MP3.hpp"

// Path to log file
#define LOG_FILE "/switch/TriPlayer/application.log"

int main(void) {
    // Start services
    romfsInit();
    socketInitializeDefault();
    Utils::MP3::init();

    // Start logging
    Log::openFile(LOG_FILE, Log::Level::Success);
    Log::writeSuccess("=== Application Launched ===");

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