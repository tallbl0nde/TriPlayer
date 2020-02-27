#include "Application.hpp"
#include "Utils.hpp"
#include "Socket.hpp"

int main(void) {
    #if _NXLINK_
        socketInitializeDefault();
        nxlinkStdio();
        Utils::writeStdout("=== stdout redirected to nxlink ===");
    #endif

    // Start services
    romfsInit();

    SockFD sfd = Utils::Socket::createSocket(3333);
    Utils::Socket::writeToSocket(sfd, "Hello World");
    Utils::Socket::readFromSocket(sfd);
    Utils::Socket::closeSocket(sfd);
    Utils::Socket::closeSocket(sfd);

    // Start actual 'app' execution
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