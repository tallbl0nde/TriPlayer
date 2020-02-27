#include "Application.hpp"
#include <iostream>

int main(void) {

#if _NXLINK_
    socketInitializeDefault();
    nxlinkStdio();
    std::cout << "=== stdout redirected to nxlink ===" << std::endl;
#endif

    // Start services
    romfsInit();

    Main::Application * app = new Main::Application();
    app->run();
    delete app;

    // Stop services
    romfsExit();

#if _NXLINK_
    socketExit();
#endif

    return 0;
}