#include "Application.hpp"
#include "ui/frame/settings/AppGeneral.hpp"

namespace Frame::Settings {
    AppGeneral::AppGeneral(Main::Application * a) : Frame(a) {
        Config * cfg = this->app->config();

        // General::confirm_clear_queue
        this->addToggle("Confirm Clearing Queue", [cfg]() -> bool {
            return cfg->confirmClearQueue();
        }, [cfg](bool b) {
            cfg->setConfirmClearQueue(b);
        });
        this->addComment("Show a dialog to confirm any actions that will clear the current play queue.");

        // General::confirm_exit
        this->addToggle("Confirm Exit", [cfg]() -> bool {
            return cfg->confirmExit();
        }, [cfg](bool b) {
            cfg->setConfirmExit(b);
        });
        this->addComment("Show a dialog to confirm you want to exit the app.");

        // General::initial_frame

        // General::log_level

        // General::scan_on_launch
        this->addToggle("Scan Library on Launch", [cfg]() -> bool {
            return cfg->scanOnLaunch();
        }, [cfg](bool b) {
            cfg->setScanOnLaunch(b);
        });
        this->addComment("This should remain enabled unless you have a really large library that doesn't change and the initial scan takes too long. No support will be given if this option is disabled, as an out-of-date database will cause bad things to happen.");

        // General::set_queue_max
    }
};