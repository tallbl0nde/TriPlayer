#include "Application.hpp"

int main(void) {
    Main::Application * app = new Main::Application();
    app->run();
    delete app;
}