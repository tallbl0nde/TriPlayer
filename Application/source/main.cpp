#include "Application.hpp"

int main(void) {
    romfsInit();
    Main::Application * app = new Main::Application();
    app->run();
    delete app;
    romfsExit();
}