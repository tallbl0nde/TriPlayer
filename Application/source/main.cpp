#include "Application.hpp"
#include "Log.hpp"

// Path to log file
#define LOG_FILE "/switch/TriPlayer/application.log"

int main(void) {
    // Start services
    romfsInit();
    socketInitializeDefault();

    // Start logging
    Log::openFile(LOG_FILE, Log::Level::Success);
    Log::writeSuccess("=== Application Launched ===");

    // Start actual 'app' execution
    Main::Application * app = new Main::Application();
    app->run();
    delete app;

    // Stop services
    socketExit();
    romfsExit();

    return 0;
}