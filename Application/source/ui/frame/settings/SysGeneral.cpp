#include "Application.hpp"
#include "lang/Lang.hpp"
#include "ui/frame/settings/SysGeneral.hpp"
#include "utils/Utils.hpp"

// Convert between Aether::Button and NX::Button (relies on order in enum - so kinda risky)
std::vector<NX::Button> aetherToNXButton(const std::vector<Aether::Button> & aether) {
    std::vector<NX::Button> nx;
    for (Aether::Button b : aether) {
        if (b <= Aether::Button::DPAD_DOWN) {
            nx.push_back(static_cast<NX::Button>(b));
        }
    }
    return nx;
}

// Convert between NX::Button and Aether::Button (relies on order in enum - so kinda risky)
std::vector<Aether::Button> nxToAetherButton(const std::vector<NX::Button> & nx) {
    std::vector<Aether::Button> aether;
    for (NX::Button b : nx) {
        aether.push_back(static_cast<Aether::Button>(b));
    }
    return aether;
}

namespace Frame::Settings {
    SysGeneral::SysGeneral(Main::Application * a) : Frame(a) {
        // Temporary variables
        Config * cfg = this->app->config();
        Aether::ListOption * opt;

        // General::pause_on_sleep
        this->addToggle("Settings.SysGeneral.PauseOnSleep"_lang, [cfg]() -> bool {
            return cfg->sysPauseOnSleep();
        }, [this, cfg](bool b) {
            cfg->setSysPauseOnSleep(b);
            this->app->sysmodule()->sendReloadConfig();
        });

        // General::pause_on_unplug
        this->addToggle("Settings.SysGeneral.PauseOnUnplug"_lang, [cfg]() -> bool {
            return cfg->sysPauseOnUnplug();
        }, [this, cfg](bool b) {
            cfg->setSysPauseOnUnplug(b);
            this->app->sysmodule()->sendReloadConfig();
        });
        this->addComment("Settings.SysGeneral.PauseText"_lang);
        this->list->addElement(new Aether::ListSeparator());

        // General::key_combo_enabled
        this->addToggle("Settings.SysGeneral.AdjustPlayback"_lang, [cfg]() -> bool {
            return cfg->sysKeyComboEnabled();
        }, [this, cfg](bool b) {
            cfg->setSysKeyComboEnabled(b);
            this->app->sysmodule()->sendReloadConfig();
        });

        // General::key_combo_next
        opt = new Aether::ListOption("Settings.SysGeneral.NextTrack"_lang, NX::comboToUnicodeString(cfg->sysKeyComboNext(), " + "), nullptr);
        opt->onPress([this, cfg, opt]() {
            this->showPickCombo("Settings.SysGeneral.NextTrackCombination"_lang, opt, [cfg]() -> std::vector<NX::Button> {
                return cfg->sysKeyComboNext();
            }, [cfg](const std::vector<NX::Button> & combo) -> bool {
                return cfg->setSysKeyComboNext(combo);
            });
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), this->app->theme()->accent());
        this->list->addElement(opt);

        // General::key_combo_play
        opt = new Aether::ListOption("Settings.SysGeneral.PlayPause"_lang, NX::comboToUnicodeString(cfg->sysKeyComboPlay(), " + "), nullptr);
        opt->onPress([this, cfg, opt]() {
            this->showPickCombo("Settings.SysGeneral.PlayPauseCombination"_lang, opt, [cfg]() -> std::vector<NX::Button> {
                return cfg->sysKeyComboPlay();
            }, [cfg](const std::vector<NX::Button> & combo) -> bool {
                return cfg->setSysKeyComboPlay(combo);
            });
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), this->app->theme()->accent());
        this->list->addElement(opt);

        // General::key_combo_prev
        opt = new Aether::ListOption("Settings.SysGeneral.PrevTrack"_lang, NX::comboToUnicodeString(cfg->sysKeyComboPrev(), " + "), nullptr);
        opt->onPress([this, cfg, opt]() {
            this->showPickCombo("Settings.SysGeneral.PrevTrackCombination"_lang, opt, [cfg]() -> std::vector<NX::Button> {
                return cfg->sysKeyComboPrev();
            }, [cfg](const std::vector<NX::Button> & combo) -> bool {
                return cfg->setSysKeyComboPrev(combo);
            });
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), this->app->theme()->accent());
        this->list->addElement(opt);
        this->addComment("Settings.SysGeneral.AdjustPlaybackText"_lang);
        this->list->addElement(new Aether::ListSeparator());

        // General::log_level
        opt = new Aether::ListOption("Settings.SysGeneral.LoggingLevel"_lang, Log::levelToString(cfg->sysLogLevel()), nullptr);
        opt->onPress([this, opt]() {
            this->showLogLevelList(opt);
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), this->app->theme()->accent());
        this->list->addElement(opt);
        this->addComment("Settings.SysGeneral.LoggingLevelText"_lang);
        this->list->addElement(new Aether::ListSeparator());

        // Restart sysmodule
        this->addButton("Settings.SysGeneral.RestartSysmodule"_lang, [this]() {
            if (this->app->sysmodule()->terminate()) {
                this->app->sysmodule()->launch();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                this->app->sysmodule()->reconnect();
            }
        });
        this->addComment("Settings.SysGeneral.RestartSysmoduleText"_lang);

        // Stop sysmodule
        this->addButton("Settings.SysGeneral.StopSysmodule"_lang, [this]() {
            // Only close the app if it was actually successful
            if (this->app->sysmodule()->terminate()) {
                this->app->exit(true);
            }
        });
        this->addComment("Settings.SysGeneral.StopSysmoduleText"_lang);

        // Overlays
        this->ovlCombo = new CustomOvl::ComboPicker();
        this->ovlCombo->setBackgroundColour(this->app->theme()->popupBG());
        this->ovlCombo->setButtonActiveColour(this->app->theme()->accent());
        this->ovlCombo->setButtonInactiveColour(this->app->theme()->FG());
        this->ovlCombo->setLineColour(this->app->theme()->muted());
        this->ovlCombo->setMutedTextColour(this->app->theme()->muted());
        this->ovlCombo->setTextColour(this->app->theme()->FG());
        this->ovlCombo->setBackLabel("Common.Back"_lang);
        this->ovlCombo->setOKLabel("Common.OK"_lang);
        this->ovlCombo->setRemoveLabel("Common.Remove"_lang);
        this->ovlCombo->setTipText("Settings.SysGeneral.ComboHint"_lang);

        this->ovlList = new Aether::PopupList("");
        this->ovlList->setBackLabel("Common.Back"_lang);
        this->ovlList->setOKLabel("Common.OK"_lang);
        this->ovlList->setBackgroundColour(this->app->theme()->popupBG());
        this->ovlList->setHighlightColour(this->app->theme()->accent());
        this->ovlList->setLineColour(this->app->theme()->muted());
        this->ovlList->setListLineColour(this->app->theme()->muted2());
        this->ovlList->setTextColour(this->app->theme()->FG());
    }

    void SysGeneral::showLogLevelList(Aether::ListOption * opt) {
        this->ovlList->setTitleLabel("Settings.SysGeneral.LoggingLevel"_lang);
        this->ovlList->removeEntries();

        // Get log levels for creation
        Log::Level array[5] = {Log::Level::Info, Log::Level::Success, Log::Level::Warning, Log::Level::Error, Log::Level::None};
        Log::Level current = this->app->config()->sysLogLevel();

        // Add entries
        for (size_t i = 0; i < 5; i++) {
            Log::Level level = array[i];
            std::string str = Log::levelToString(level);
            this->ovlList->addEntry(str, [this, opt, str, level]() {
                opt->setValue(str);
                this->app->config()->setSysLogLevel(level);
                this->app->sysmodule()->sendReloadConfig();
            }, current == level);
        }

        this->app->addOverlay(this->ovlList);
    }

    void SysGeneral::showPickCombo(const std::string & title, Aether::ListOption * opt, std::function<std::vector<NX::Button>()> get, std::function<bool(std::vector<NX::Button>)> set) {
        this->ovlCombo->setTitleText(title);
        this->ovlCombo->setCombo(nxToAetherButton(get()));
        this->ovlCombo->setCallback([this, opt, set](std::vector<Aether::Button> combo) {
            // Convert to NX::Button
            std::vector<NX::Button> nxCombo = aetherToNXButton(combo);
            nxCombo = Utils::removeDuplicates(nxCombo);
            if (nxCombo.empty()) {
                return;
            }

            // Otherwise update config and only change if written successfully
            if (set(nxCombo)) {
                opt->setValue(NX::comboToUnicodeString(nxCombo, " + "));
                this->app->sysmodule()->sendReloadConfig();
            }
        });
        this->app->addOverlay(this->ovlCombo);
    }

    SysGeneral::~SysGeneral() {
        delete this->ovlCombo;
        delete this->ovlList;
    }
};
