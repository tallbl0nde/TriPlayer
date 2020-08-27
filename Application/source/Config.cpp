#include "Config.hpp"
#include "minIni.h"
#include "utils/FS.hpp"
#include "utils/Utils.hpp"

Config::Config(const std::string & path) {
    // Check the file exists and copy template if it doesn't
    if (!Utils::Fs::fileExists(path)) {
        if (!Utils::Fs::copyFile("romfs:/config/app_config.ini", path)) {
            Log::writeError("[CONFIG] Unable to copy template, bad things may happen");
        }
    }

    this->ini = new minIni(path);
    this->sysIni = nullptr;
}

bool Config::prepareSys(const std::string & sysPath) {
    if (!Utils::Fs::fileExists(sysPath)) {
        Log::writeError("[CONFIG] An .ini does not exist at the specified sysPath");
        return false;
    }

    delete this->sysIni;
    this->sysIni = new minIni(sysPath);
    return true;
}

int Config::version() {
    int version = this->ini->geti("Version", "version", -1);
    if (version < 0) {
        Log::writeError("[CONFIG] Failed to get .ini version");
    }
    return version;
}

bool Config::setVersion(const int v) {
    bool ok = this->ini->put("Version", "version", v);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set .ini version");
    }
    return ok;
}

bool Config::confirmClearQueue() {
    return this->ini->getbool("General", "confirm_clear_queue");
}

bool Config::setConfirmClearQueue(const bool b) {
    bool ok = this->ini->put("General", "confirm_clear_queue", (b ? "Yes" : "No"));
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set confirm_clear_queue");
    }
    return ok;
}

bool Config::confirmExit() {
    return this->ini->getbool("General", "confirm_exit");
}

bool Config::setConfirmExit(const bool b) {
    bool ok = this->ini->put("General", "confirm_exit", (b ? "Yes" : "No"));
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set confirm_exit");
    }
    return ok;
}

Frame::Type Config::initialFrame() {
    const std::string str = this->ini->gets("General", "initial_frame");
    if (str.empty()) {
        Log::writeError("[CONFIG] Failed to get initial_frame");
        return Frame::Type::Songs;
    }

    if (str == "Playlists") {
        return Frame::Type::Playlists;

    } else if (str == "Albums") {
        return Frame::Type::Albums;

    } else if (str == "Artists") {
        return Frame::Type::Artists;
    }

    // If some other value just return Songs
    return Frame::Type::Songs;
}

bool Config::setInitialFrame(const Frame::Type t) {
    std::string str = "";
    switch (t) {
        case Frame::Type::Songs:
            str = "Songs";
            break;

        case Frame::Type::Playlists:
            str = "Playlists";
            break;

        case Frame::Type::Albums:
            str = "Albums";
            break;

        case Frame::Type::Artists:
            str = "Artists";
            break;

        default:
            Log::writeError("[CONFIG] Invalid frame type passed to initial_frame");
            return false;
    }

    bool ok = this->ini->put("General", "initial_frame", str);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set initial_frame");
    }
    return ok;
}

Log::Level Config::logLevel() {
    std::string str = this->ini->gets("General", "log_level");
    if (str.empty()) {
        Log::writeError("[CONFIG] Failed to get log_level");
        return Log::Level::Warning;
    }

    if (str == "None") {
        return Log::Level::None;

    } else if (str == "Success") {
        return Log::Level::Success;

    } else if (str == "Error") {
        return Log::Level::Error;

    } else if (str == "Info") {
        return Log::Level::Info;

    }

    // Return Warning if some unknown value
    return Log::Level::Warning;
}

bool Config::setLogLevel(const Log::Level l) {
    std::string str;
    switch (l) {
        case Log::Level::None:
            str = "None";
            break;

        case Log::Level::Success:
            str = "Success";
            break;

        case Log::Level::Warning:
            str = "Warning";
            break;

        case Log::Level::Error:
            str = "Error";
            break;

        case Log::Level::Info:
            str = "Info";
            break;
    }

    bool ok = this->ini->put("General", "log_level", str);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set log_level");
    }
    return ok;
}

bool Config::scanOnLaunch() {
    return this->ini->getbool("General", "scan_on_launch");
}

bool Config::setScanOnLaunch(const bool b) {
    bool ok = this->ini->put("General", "scan_on_launch", (b ? "Yes" : "No"));
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set scan_on_launch");
    }
    return ok;
}

int Config::setQueueMax() {
    int max = this->ini->geti("General", "set_queue_max", -42069);
    if (max < -1) {
        Log::writeError("[CONFIG] Failed to get set_queue_max");
        return -1;
    }
    return max;
}

bool Config::setSetQueueMax(const int v) {
    bool ok = this->ini->put("General", "set_queue_max", v);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set set_queue_max");
    }
    return ok;
}

// AccentColour Config::accentColour() {

// }

// bool Config::setAccentColour(AccentColour) {
//
//}

bool Config::showTouchControls() {
    return this->ini->getbool("General", "show_touch_controls");
}

bool Config::setShowTouchControls(const bool b) {
    bool ok = this->ini->put("Appearance", "show_touch_controls", (b ? "Yes" : "No"));
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Appearance) show_touch_controls");
    }
    return ok;
}

int Config::searchMaxPlaylists() {
    int max = this->ini->geti("Search", "max_playlists", -42069);
    if (max < -1) {
        Log::writeError("[CONFIG] Failed to get (Search) max_playlists");
        return -1;
    }
    return max;
}

bool Config::setSearchMaxPlaylists(const int i) {
    bool ok = this->ini->put("Search", "max_playlists", i);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Search) max_playlists");
    }
    return ok;
}

int Config::searchMaxArtists() {
    int max = this->ini->geti("Search", "max_artists", -42069);
    if (max < -1) {
        Log::writeError("[CONFIG] Failed to get (Search) max_artists");
        return -1;
    }
    return max;
}

bool Config::setSearchMaxArtists(const int i) {
    bool ok = this->ini->put("Search", "max_artists", i);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Search) max_artists");
    }
    return ok;
}

int Config::searchMaxAlbums() {
    int max = this->ini->geti("Search", "max_albums", -42069);
    if (max < -1) {
        Log::writeError("[CONFIG] Failed to get (Search) max_albums");
        return -1;
    }
    return max;
}

bool Config::setSearchMaxAlbums(const int i) {
    bool ok = this->ini->put("Search", "max_albums", i);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Search) max_albums");
    }
    return ok;
}

int Config::searchMaxSongs() {
    int max = this->ini->geti("Search", "max_songs", -42069);
    if (max < -1) {
        Log::writeError("[CONFIG] Failed to get (Search) max_songs");
        return -1;
    }
    return max;
}

bool Config::setSearchMaxSongs(const int i) {
    bool ok = this->ini->put("Search", "max_songs", i);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Search) max_songs");
    }
    return ok;
}

bool Config::autoLaunchService() {
    return this->ini->getbool("Advanced", "auto_launch_service");
}

bool Config::setAutoLaunchService(const bool b) {
    bool ok = this->ini->put("Advanced", "auto_launch_service", (b ? "Yes" : "No"));
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Advanced) auto_launch_service");
    }
    return ok;
}

Log::Level Config::sysLogLevel() {
    if (!this->sysIni) {
        Log::writeError("[CONFIG] Can't access sysmodule config as object was not prepared");
        return Log::Level::Warning;
    }

    std::string str = this->sysIni->gets("General", "log_level");
    if (str.empty()) {
        Log::writeError("[CONFIG] Failed to get (Sysmodule) log_level");
        return Log::Level::Warning;
    }

    if (str == "Info") {
        return Log::Level::Info;

    } else if (str == "Success") {
        return Log::Level::Success;

    } else if (str == "Error") {
        return Log::Level::Error;

    } else if (str == "None") {
        return Log::Level::None;
    }

    return Log::Level::Warning;
}

bool Config::setSysLogLevel(const Log::Level l) {
    if (!this->sysIni) {
        Log::writeError("[CONFIG] Can't access sysmodule config as object was not prepared");
        return false;
    }

    std::string str;
    switch (l) {
        case Log::Level::None:
            str = "None";
            break;

        case Log::Level::Success:
            str = "Success";
            break;

        case Log::Level::Warning:
            str = "Warning";
            break;

        case Log::Level::Error:
            str = "Error";
            break;

        case Log::Level::Info:
            str = "Info";
            break;
    }

    bool ok = this->sysIni->put("General", "log_level", str);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Sysmodule) log_level");
    }
    return ok;
}

bool Config::sysMP3AccurateSeek() {
    if (!this->sysIni) {
        Log::writeError("[CONFIG] Can't access sysmodule config as object was not prepared");
        return false;
    }

    return this->sysIni->getbool("MP3", "accurate_seek");
}

bool Config::setSysMP3AccurateSeek(const bool b) {
    if (!this->sysIni) {
        Log::writeError("[CONFIG] Can't access sysmodule config as object was not prepared");
        return false;
    }

    bool ok = this->sysIni->put("MP3", "accurate_seek", (b ? "Yes" : "No"));
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Sysmodule) accurate_seek");
    }
    return ok;
}

std::array<float, 32> Config::sysMP3Equalizer() {
    // Default array to return
    std::array<float, 32> eq;
    eq.fill(1.0f);
    size_t idx;

    // Ensure we can read from the sysmodule config
    if (!this->sysIni) {
        Log::writeError("[CONFIG] Can't access sysmodule config as object was not prepared");
        return eq;
    }

    // Iterate over each row
    std::string key;
    for (size_t row = 0; row < 4; row++) {
        // Get the value for the row
        key = "equalizer_" + std::to_string((row*8) + 1) + "_" + std::to_string((row+1) * 8);
        const std::string str = this->sysIni->gets("MP3", key, "");
        char * cstr = strdup(str.c_str());

        // Split on comma/space and push value on array
        idx = row*8;
        char * tok = strtok(cstr, ", ");
        while (tok != nullptr && idx < (row+1)*8) {
            eq[idx] = strtol(tok, nullptr, 10);
            idx++;
            tok = strtok(nullptr, ", ");
        }
        free(cstr);
    }

    return eq;
}

bool Config::setSysMP3Equalizer(const std::array<float, 32> & eq) {
    if (!this->sysIni) {
        Log::writeError("[CONFIG] Can't access sysmodule config as object was not prepared");
        return false;
    }

    // Construct strings from array
    std::array<std::string, 4> rows;
    for (size_t i = 0; i < eq.size(); i++) {
        size_t idx = i/8;
        rows[idx] += Utils::truncateToDecimalPlace(std::to_string(Utils::roundToDecimalPlace(eq[i], 2)), 2);
        if (i%8 != 7) {
            rows[idx] += ", ";
        }
    }

    // Write each string to file
    for (size_t i = 0; i < rows.size(); i++) {
        std::string key = "equalizer_" + std::to_string((i*8)+1) + "_" + std::to_string((i+1)*8);
        bool ok = this->sysIni->put("MP3", key, rows[i]);
        if (!ok) {
            Log::writeError("[CONFIG] Failed to set (Sysmodule) MP3 equalizer row " + std::to_string(i));
            return false;
        }
    }

    return true;
}

Config::~Config() {
    delete this->ini;
    delete this->sysIni;
}
