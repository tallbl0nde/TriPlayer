#include "Application.hpp"
#include "ui/frame/Frame.hpp"
#include "ui/frame/settings/AppGeneral.hpp"
#include "utils/NX.hpp"

// Helper to convert Frame type to string
static std::string frameToString(Frame::Type f) {
    std::string str = "?";
    switch (f) {
        case Frame::Type::Playlists:
            str = "Playlists";
            break;

        case Frame::Type::Albums:
            str = "Albums";
            break;

        case Frame::Type::Artists:
            str = "Artists";
            break;

        case Frame::Type::Songs:
            str = "Songs";
            break;

        default:
            break;
    }
    return str;
}

namespace Frame::Settings {
    AppGeneral::AppGeneral(Main::Application * a) : Frame(a) {
        // Temporary variables
        Config * cfg = this->app->config();
        Aether::ListOption * opt;

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
        opt = new Aether::ListOption("Initial Section", frameToString(cfg->initialFrame()), nullptr);
        opt->setCallback([this, opt]() {
            this->showInitialFrameList(opt);
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), this->app->theme()->accent());
        this->list->addElement(opt);
        this->addComment("The section to show when the app is launched.");

        // General::log_level
        opt = new Aether::ListOption("Logging Level", Log::levelToString(cfg->logLevel()), nullptr);
        opt->setCallback([this, opt]() {
            this->showLogLevelList(opt);
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), this->app->theme()->accent());
        this->list->addElement(opt);
        this->addComment("This only adjusts the application's log level, not the sysmodule's. Each level will log it and the levels below (e.g. Warning will log both Warning and Error messages). Info should only be used for debugging purposes, as it logs a LOT of information and slows down the app.");

        // General::scan_on_launch
        this->addToggle("Scan Library on Launch", [cfg]() -> bool {
            return cfg->scanOnLaunch();
        }, [cfg](bool b) {
            cfg->setScanOnLaunch(b);
        });
        this->addComment("This should remain enabled unless you have a really large library that doesn't change and the initial scan takes too long. No support will be given if this option is disabled, as an out-of-date database will cause bad things to happen.");

        // General::set_queue_max
        opt = new Aether::ListOption("Initial Queue Size", std::to_string(cfg->setQueueMax()), nullptr);
        opt->setCallback([this, cfg, opt]() {
            int val = cfg->setQueueMax();
            if (this->getNumberInput(val, "Initial Queue Size", "", true)) {
                val = (val < -1 ? -1 : (val > 65535 ? 65535 : val));
                if (cfg->setSetQueueMax(val)) {
                    opt->setValue(std::to_string(val));
                    this->app->sysmodule()->setQueueLimit(val);
                }
            }
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), this->app->theme()->accent());
        this->list->addElement(opt);
        this->addComment("Number of songs to create a queue with when playing a song/album/etc. A negative number indicates no limit.");

        // Overlays
        this->ovlList = new Aether::PopupList("");
        this->ovlList->setBackLabel("Back");
        this->ovlList->setOKLabel("OK");
        this->ovlList->setBackgroundColour(this->app->theme()->popupBG());
        this->ovlList->setHighlightColour(this->app->theme()->accent());
        this->ovlList->setLineColour(this->app->theme()->muted());
        this->ovlList->setListLineColour(this->app->theme()->muted2());
        this->ovlList->setTextColour(this->app->theme()->FG());
    }

    void AppGeneral::showInitialFrameList(Aether::ListOption * opt) {
        this->ovlList->setTitleLabel("Initial Frame");
        this->ovlList->removeEntries();

        // Get frame types for creation
        Type array[4] = {Type::Playlists, Type::Albums, Type::Artists, Type::Songs};
        Type current = this->app->config()->initialFrame();

        // Add entries
        for (size_t i = 0; i < 4; i++) {
            Type type = array[i];
            std::string str = frameToString(type);
            this->ovlList->addEntry(str, [this, opt, str, type]() {
                opt->setValue(str);
                this->app->config()->setInitialFrame(type);
            }, current == type);
        }

        this->app->addOverlay(this->ovlList);
    }

    void AppGeneral::showLogLevelList(Aether::ListOption * opt) {
        this->ovlList->setTitleLabel("Logging Level");
        this->ovlList->removeEntries();

        // Get log levels for creation
        Log::Level array[5] = {Log::Level::Info, Log::Level::Success, Log::Level::Warning, Log::Level::Error, Log::Level::None};
        Log::Level current = this->app->config()->logLevel();

        // Add entries
        for (size_t i = 0; i < 5; i++) {
            Log::Level level = array[i];
            std::string str = Log::levelToString(level);
            this->ovlList->addEntry(str, [this, opt, str, level]() {
                if (this->app->config()->setLogLevel(level)) {
                    opt->setValue(str);
                    Log::setLogLevel(level);
                }
            }, current == level);
        }

        this->app->addOverlay(this->ovlList);
    }

    AppGeneral::~AppGeneral() {
        delete this->ovlList;
    }
};