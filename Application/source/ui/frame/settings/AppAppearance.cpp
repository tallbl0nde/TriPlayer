#include "Application.hpp"
#include "lang/Lang.hpp"
#include "ui/frame/settings/AppAppearance.hpp"

namespace Frame::Settings {
    AppAppearance::AppAppearance(Main::Application * a) : Frame(a) {
        // Temporary variables
        Config * cfg = this->app->config();
        Aether::ListOption * opt;

        // Appearance::accent_colour
        opt = new Aether::ListOption("Settings.AppAppearance.AccentColour"_lang, Theme::colourToString(cfg->accentColour()), nullptr);
        opt->setCallback([this, opt]() {
            this->showAccentColourList(opt);
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), this->app->theme()->accent());
        this->list->addElement(opt);
        this->addComment("Settings.AppAdvanced.AccentColourText"_lang);

        // Appearance::auto_player_palette
        this->addToggle("Settings.AppAdvanced.AutoPlayerPalette"_lang, [cfg]() -> bool {
            return cfg->autoPlayerPalette();
        }, [cfg](bool b) {
            cfg->setAutoPlayerPalette(b);
        });
        this->addComment("Settings.AppAdvanced.AutoPlayerPaletteText"_lang);
        this->list->addElement(new Aether::ListSeparator());

        // Appearance::show_touch_controls
        this->addToggle("Settings.AppAdvanced.ShowTouchControls"_lang, [cfg]() -> bool {
            return cfg->showTouchControls();
        }, [cfg](bool b) {
            cfg->setShowTouchControls(b);
        });
        this->addComment("Settings.AppAdvanced.ShowTouchControlsText"_lang);

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

    void AppAppearance::showAccentColourList(Aether::ListOption * opt) {
        this->ovlList->setTitleLabel("Settings.AppAppearance.AccentColour"_lang);
        this->ovlList->removeEntries();

        // Get colours for creation
        Theme::Colour array[7] = {Theme::Colour::Red, Theme::Colour::Orange, Theme::Colour::Yellow, Theme::Colour::Green, Theme::Colour::Blue, Theme::Colour::Purple, Theme::Colour::Pink};
        Theme::Colour current = this->app->config()->accentColour();

        // Add entries
        for (size_t i = 0; i < 7; i++) {
            Theme::Colour col = array[i];
            std::string str = Theme::colourToString(col);
            this->ovlList->addEntry(str, [this, opt, str, col]() {
                // Update config
                opt->setValue(str);
                this->app->config()->setAccentColour(col);

                // Actually update colours
                this->app->theme()->setAccent(col);
                opt->setValueColour(this->app->theme()->accent());
                this->ovlList->setHighlightColour(this->app->theme()->accent());
                this->app->setHighlightAnimation(nullptr);
                this->app->updateScreenTheme();
            }, current == col);
        }

        this->app->addOverlay(this->ovlList);
    }

    AppAppearance::~AppAppearance() {
        delete this->ovlList;
    }
};