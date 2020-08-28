#include "Application.hpp"
#include "ui/frame/settings/AppAdvanced.hpp"

namespace Frame::Settings {
    AppAdvanced::AppAdvanced(Main::Application * a) : Frame(a) {
        // Temporary variables
        Config * cfg = this->app->config();

        // Advanced::auto_launch_service
        this->addToggle("Auto Launch Sysmodule", [cfg]() -> bool {
            return cfg->autoLaunchService();
        }, [cfg](bool b) {
            cfg->setAutoLaunchService(b);
        });
        this->addComment("Automatically attempt to start the sysmodule if it is not running when the app is launched.");

        // Scan now
        this->addButton("Scan Now", [this]() {
            // fix this
            // this->app->popScreen();
            // this->app->setScreen(Main::ScreenID::Splash);
        });

        // Search for artist images
        this->addButton("Search for Missing Artist Images", [this]() {
            // perform some magic
        });
    }
};