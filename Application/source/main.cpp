#include "Application.hpp"
#include "Utils.hpp"
#include "Sysmodule.hpp"

int main(void) {
    #if _NXLINK_
        socketInitializeDefault();
        nxlinkStdio();
        Utils::writeStdout("=== stdout redirected to nxlink ===");
    #endif

    // Start services
    romfsInit();

    Sysmodule * sm = new Sysmodule();

    // Start actual 'app' execution
    Main::Application * app = new Main::Application();
    app->run();
    delete app;

    delete sm;

    // Stop services
    romfsExit();

    #if _NXLINK_
        socketExit();
    #endif

    return 0;
}