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
    }
};