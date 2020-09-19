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
        this->list->addElement(new Aether::ListSeparator());

        // General::initial_frame
        opt = new Aether::ListOption("Initial Section", frameToString(cfg->initialFrame()), nullptr);
        opt->setCallback([this, opt]() {
            this->showInitialFrameList(opt);
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), this->app->theme()->accent());
        this->list->addElement(opt);
        this->addComment("The section to show when the app is launched.");

        // General::skip_with_lr
        this->addToggle("Skip with L/R", [cfg]() -> bool {
            return cfg->skipWithLR();
        }, [cfg](bool b) {
            cfg->setSkipWithLR(b);
        });
        this->addComment("Use L/R to skip backwards/forwards in the queue. This is only functional within the application.");
        this->list->addElement(new Aether::ListSeparator());

        // General::log_level
        opt = new Aether::ListOption("Logging Level", Log::levelToString(cfg->logLevel()), nullptr);
        opt->setCallback([this, opt]() {
            this->showLogLevelList(opt);
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), this->app->theme()->accent());
        this->list->addElement(opt);
        this->addComment("This only adjusts the application's log level, not the sysmodule's. Each level will log it and the levels below (e.g. Warning will log both Warning and Error messages). Info should only be used for debugging purposes, as it logs a LOT of information and slows down the app.");

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