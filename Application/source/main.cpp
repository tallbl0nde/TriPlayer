#include "Application.hpp"
#include "Utils.hpp"

int main(void) {
    // Start services
    romfsInit();
    socketInitializeDefault();

    #if _NXLINK_
        nxlinkStdio();
        Utils::writeStdout("=== stdout redirected to nxlink ===");
    #endif

    // Start actual 'app' execution
    Main::Application * app = new Main::Application();
    app->run();
    delete app;

    // Stop services
    romfsExit();
    socketExit();

    return 0;
}