#include "Application.hpp"
#include "ui/frame/settings/SysMP3.hpp"

namespace Frame::Settings {
    SysMP3::SysMP3(Main::Application * a) : Frame(a) {
        // Temporary variables
        Config * cfg = this->app->config();

        // MP3::accurate_seek
        this->addToggle("Accurate Seek", [cfg]() -> bool {
            return cfg->sysMP3AccurateSeek();
        }, [this, cfg](bool b) {
            cfg->setSysMP3AccurateSeek(b);
            this->app->sysmodule()->sendReloadConfig();
        });
        this->addComment("This is recommended to be disabled as accurate seeking is quite slow. 'Fuzzy' seeking isn't 100% accurate but is good enough for most tracks.");

        // Equalizer
        this->addButton("Equalizer", [this]() {
            this->showEqualizer();
        });

        // Setup overlay
        this->ovlEQ = new CustomOvl::Equalizer("Equalizer");
        this->ovlEQ->setApplyLabel("Apply");
        this->ovlEQ->setBackLabel("Back");
        this->ovlEQ->setOKLabel("OK");
        this->ovlEQ->setResetLabel("Reset");
        this->ovlEQ->setBackgroundColour(this->app->theme()->popupBG());
        this->ovlEQ->setHeadingColour(this->app->theme()->FG());
        this->ovlEQ->setLineColour(this->app->theme()->muted());
        this->ovlEQ->setSliderBackgroundColour(this->app->theme()->muted2());
        this->ovlEQ->setSliderForegroundColour(this->app->theme()->accent());
        this->ovlEQ->setSliderKnobColour(this->app->theme()->FG());
        this->ovlEQ->setApplyCallback([this]() {
            std::array<float, 32> arr = this->ovlEQ->getValues();
            this->app->config()->setSysMP3Equalizer(arr);
            this->app->sysmodule()->sendReloadConfig();
        });
        this->ovlEQ->setResetCallback([this]() {
            std::array<float, 32> arr;
            arr.fill(1.0f);
            this->ovlEQ->setValues(arr);
        });
    }

    void SysMP3::showEqualizer() {
        // Set initial values and add
        std::array<float, 32> arr = this->app->config()->sysMP3Equalizer();
        this->ovlEQ->setValues(arr);
        this->app->addOverlay(this->ovlEQ);
    }

    SysMP3::~SysMP3() {
        delete this->ovlEQ;
    }
};