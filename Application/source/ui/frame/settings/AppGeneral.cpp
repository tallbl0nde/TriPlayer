#include "Application.hpp"
#include "lang/Lang.hpp"
#include "ui/frame/Frame.hpp"
#include "ui/frame/settings/AppGeneral.hpp"
#include "utils/NX.hpp"

// Helper to convert Frame type to string
static std::string frameToString(Frame::Type f) {
    std::string str = "?";
    switch (f) {
        case Frame::Type::Playlists:
            str = "Playlist.Playlists"_lang;
            break;

        case Frame::Type::Albums:
            str = "Album.Albums"_lang;
            break;

        case Frame::Type::Artists:
            str = "Artist.Artists"_lang;
            break;

        case Frame::Type::Songs:
            str = "Song.Songs"_lang;
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
        this->addToggle("Settings.AppGeneral.ConfirmClearingQueue"_lang, [cfg]() -> bool {
            return cfg->confirmClearQueue();
        }, [cfg](bool b) {
            cfg->setConfirmClearQueue(b);
        });
        this->addComment("Settings.AppGeneral.ConfirmClearingQueueText"_lang);

        // General::confirm_exit
        this->addToggle("Settings.AppGeneral.ConfirmExit"_lang, [cfg]() -> bool {
            return cfg->confirmExit();
        }, [cfg](bool b) {
            cfg->setConfirmExit(b);
        });
        this->addComment("Settings.AppGeneral.ConfirmExitText"_lang);
        this->list->addElement(new Aether::ListSeparator());

        // General::initial_frame
        opt = new Aether::ListOption("Settings.AppGeneral.InitialSection"_lang, frameToString(cfg->initialFrame()), nullptr);
        opt->setCallback([this, opt]() {
            this->showInitialFrameList(opt);
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), this->app->theme()->accent());
        this->list->addElement(opt);
        this->addComment("Settings.AppGeneral.InitialSectionText"_lang);

        // General::skip_with_lr
        this->addToggle("Settings.AppGeneral.SkipWithLR"_lang, [cfg]() -> bool {
            return cfg->skipWithLR();
        }, [cfg](bool b) {
            cfg->setSkipWithLR(b);
        });
        this->addComment("Settings.AppGeneral.SkipWithLRText"_lang);
        this->list->addElement(new Aether::ListSeparator());

        // General::log_level
        opt = new Aether::ListOption("Settings.AppGeneral.LoggingLevel"_lang, Log::levelToString(cfg->logLevel()), nullptr);
        opt->setCallback([this, opt]() {
            this->showLogLevelList(opt);
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), this->app->theme()->accent());
        this->list->addElement(opt);
        this->addComment("Settings.AppGeneral.LoggingLevelText"_lang);

        // Overlays
        this->ovlList = new Aether::PopupList("");
        this->ovlList->setBackLabel("Common.Back"_lang);
        this->ovlList->setOKLabel("Common.OK"_lang);
        this->ovlList->setBackgroundColour(this->app->theme()->popupBG());
        this->ovlList->setHighlightColour(this->app->theme()->accent());
        this->ovlList->setLineColour(this->app->theme()->muted());
        this->ovlList->setListLineColour(this->app->theme()->muted2());
        this->ovlList->setTextColour(this->app->theme()->FG());
    }

    void AppGeneral::showInitialFrameList(Aether::ListOption * opt) {
        this->ovlList->setTitleLabel("Settings.AppGeneral.InitialSection"_lang);
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
        this->ovlList->setTitleLabel("Settings.AppGeneral.LoggingLevel"_lang);
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