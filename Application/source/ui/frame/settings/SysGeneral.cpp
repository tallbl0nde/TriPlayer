#include "Application.hpp"
#include "ui/frame/settings/SysGeneral.hpp"

namespace Frame::Settings {
    SysGeneral::SysGeneral(Main::Application * a) : Frame(a) {
        // Temporary variables
        Config * cfg = this->app->config();
        Aether::ListOption * opt;

        // General::log_level
        opt = new Aether::ListOption("Logging Level", Log::levelToString(cfg->sysLogLevel()), nullptr);
        opt->setCallback([this, opt]() {
            this->showLogLevelList(opt);
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), this->app->theme()->accent());
        this->list->addElement(opt);
        this->addComment("This only adjusts the sysmodule's log level, not the application's. Each level will log it and the levels below (e.g. Warning will log both Warning and Error messages). Info should only be used for debugging purposes, as it logs a LOT of information and slows down playback.");
        this->list->addElement(new Aether::ListSeparator());

        // Restart sysmodule
        this->addButton("Restart Sysmodule", [this]() {
            if (this->app->sysmodule()->terminate()) {
                this->app->sysmodule()->launch();
            }
        });
        this->addComment("Restart the sysmodule and wait for it to reload.");

        // Stop sysmodule
        this->addButton("Stop Sysmodule", [this]() {
            // Only close the app if it was actually successful
            if (this->app->sysmodule()->terminate()) {
                this->app->exit();
            }
        });
        this->addComment("Safely stop the sysmodule and close the application.");

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

    void SysGeneral::showLogLevelList(Aether::ListOption * opt) {
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
                opt->setValue(str);
                this->app->config()->setLogLevel(level);
                this->app->sysmodule()->sendReloadConfig();
            }, current == level);
        }

        this->app->addOverlay(this->ovlList);
    }

    SysGeneral::~SysGeneral() {
        delete this->ovlList;
    }
};