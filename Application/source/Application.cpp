#include "Application.hpp"
#include "lang/Lang.hpp"
#include "lang/Language.hpp"
#include "Paths.hpp"
#include "ui/screen/Fullscreen.hpp"
#include "ui/screen/Home.hpp"
#include "ui/screen/Settings.hpp"
#include "ui/screen/Splash.hpp"
#include "ui/screen/Update.hpp"
#include "Updater.hpp"
#include "utils/Curl.hpp"
#include "utils/NX.hpp"

// Time in seconds to wait before checking for an update automatically
constexpr size_t updateInterval = 21600;        // 6 hours

namespace Main {
    Application::Application() : database_(SyncDatabase(new Database())) {
        // Load config
        this->config_ = new Config(Path::App::ConfigFile);
        this->database_->setSpellfixScore(this->config_->searchMaxScore());
        this->database_->setSearchPhraseCount(this->config_->searchMaxPhrases());

        // Start logging
        Log::openFile(Path::App::LogFile, this->config_->logLevel());
        Log::writeWarning("=== Application Launched ===");

        // Start services
        Utils::Curl::init();

        // Prepare theme
        this->theme_ = new Theme();
        this->theme_->setAccent(this->config_->accentColour());

        // Create sysmodule object (will attempt connection)
        this->sysmodule_ = new Sysmodule();
        this->sysmodule_->setQueueLimit(this->config_->setQueueMax());
        // Continue in another thread
        this->sysThread = std::async(std::launch::async, &Sysmodule::process, this->sysmodule_);

        // Create Aether instance
        this->window = new Aether::Window("TriPlayer", 1280, 720, [](const std::string msg, const bool important) {
            if (important) {
                Log::writeError("[AETHER] " + msg);
            } else {
                Log::writeInfo("[AETHER] " + msg);
            }
        });
        this->window->setBackgroundColour(Aether::Colour{0, 0, 0, 0});
        this->window->setFont("romfs:/Quicksand.ttf");
        this->window->setFontSpacing(0.9);
        this->setHighlightAnimation(nullptr);
        this->window->setHighlightBackground(Aether::Colour{255, 255, 255, 0});
        this->window->setHighlightOverlay(this->theme_->selected());
        this->window->setFadeIn(true);
        this->window->setFadeOut(true);
        // this->window->showDebugInfo(true);
        this->exitPrompt = nullptr;

        // Setup screens
        Utils::Lang::setLanguage(this->config_->language());
        this->screens[static_cast<int>(ScreenID::Fullscreen)] = new Screen::Fullscreen(this);
        this->screens[static_cast<int>(ScreenID::Home)] = new Screen::Home(this);
        this->screens[static_cast<int>(ScreenID::Settings)] = new Screen::Settings(this);
        this->screens[static_cast<int>(ScreenID::Splash)] = new Screen::Splash(this);
        this->screens[static_cast<int>(ScreenID::Update)] = new Screen::Update(this);
        this->setScreen(ScreenID::Splash);

        // Mark that we're playing media
        Utils::NX::setPlayingMedia(true);

        // Start checking for an update
        this->hasUpdate_ = false;
        this->updateThread = std::async(std::launch::async, [this]() {
            Updater updater = Updater();
            if (updater.needsCheck(updateInterval)) {
                if (updater.checkForUpdate()) {
                    this->hasUpdate_ = updater.availableUpdate();
                }
            } else {
                this->hasUpdate_ = updater.availableUpdate();
            }
        });
    }

    void Application::createExitPrompt() {
        this->exitPrompt = new Aether::MessageBox();
        this->exitPrompt->setLineColour(this->theme_->muted2());
        this->exitPrompt->setRectangleColour(this->theme_->popupBG());
        this->exitPrompt->addLeftButton("Common.Cancel"_lang, [this]() {
            this->exitPrompt->close();
        });
        this->exitPrompt->addRightButton("Common.Exit"_lang, [this]() {
            this->window->exit();
        });
        this->exitPrompt->setTextColour(this->theme_->accent());
        Aether::Element * body = new Aether::Element(0, 0, 700);
        Aether::TextBlock * tips = new Aether::TextBlock(40, 40, "Common.ExitPrompt"_lang, 24, 620);
        tips->setColour(this->theme_->FG());
        body->addElement(tips);
        body->setH(tips->h() + 80);
        this->exitPrompt->setBody(body);
        this->exitPrompt->setBodySize(body->w(), body->h());
    }

    void Application::setHoldDelay(int i) {
        this->window->setHoldDelay(i);
    }

    void Application::setHighlightAnimation(std::function<Aether::Colour(uint32_t)> f) {
        // Set to default animation
        if (f == nullptr) {
            f = this->theme_->highlightFunc();
        }
        this->window->setHighlightAnimation(f);
    }

    void Application::addOverlay(Aether::Overlay * o) {
        this->window->addOverlay(o);
    }

    void Application::setScreen(ScreenID s) {
        this->screenID = s;
        this->window->showScreen(this->screens[static_cast<int>(s)]);
    }

    void Application::pushScreen() {
        this->screenIDs.push(this->screenID);
        this->window->pushScreen();
    }

    void Application::popScreen() {
        this->window->popScreen();
        this->screenID = this->screenIDs.top();
        this->screenIDs.pop();
    }

    void Application::updateScreenTheme() {
        for (Screen::Screen * s : this->screens) {
            s->updateColours();
        }
    }

    void Application::lockDatabase() {
        this->database_->close();
        this->sysmodule_->waitRequestDBLock();
        this->database_->openReadWrite();
    }

    void Application::unlockDatabase() {
        this->database_->close();
        this->sysmodule_->sendReleaseDBLock();
        this->database_->openReadOnly();
    }

    bool Application::hasUpdate() {
        return this->hasUpdate_;
    }

    void Application::setHasUpdate(const bool b) {
        this->hasUpdate_ = b;
    }

    Config * Application::config() {
        return this->config_;
    }

    const SyncDatabase & Application::database() {
        return this->database_;
    }

    Sysmodule * Application::sysmodule() {
        return this->sysmodule_;
    }

    Theme * Application::theme() {
        return this->theme_;
    }

    void Application::run() {
        // Do main loop
        while (this->window->loop()) {
        }
    }

    void Application::exit(bool force = false) {
        // Check if we need to display a popup
        if (!force && this->config_->confirmExit()) {
            delete this->exitPrompt;
            this->createExitPrompt();
            this->addOverlay(this->exitPrompt);

        // Otherwise just exit
        } else {
            this->window->exit();
        }
    }

    Application::~Application() {
        // Wait for update thread to terminate
        this->updateThread.get();

        // Mark that we're no longer playing media
        Utils::NX::setPlayingMedia(false);

        // Unload all loaded screens
        while (true) {
            this->screens[static_cast<int>(this->screenID)]->onUnload();
            if (this->screenIDs.empty()) {
                break;
            }

            this->screenID = this->screenIDs.top();
            this->screenIDs.pop();
        }

        // Finally delete
        for (Screen::Screen * s : this->screens) {
            delete s;
        }

        // Delete overlay
        delete this->exitPrompt;

        // Cleanup Aether after screens are deleted
        delete this->window;

        // Disconnect from sysmodule
        this->sysmodule_->exit();
        this->sysThread.get();
        delete this->sysmodule_;

        // Cleanup config object
        delete this->config_;

        // Stop services
        Utils::Curl::exit();

        // The database will be closed here as the wrapper goes out of scope
    }
};
