#include "Config.hpp"
#include "lang/Language.hpp"
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
    this->readConfig();

    this->sysIni = nullptr;
}

void Config::readConfig() {
    // Temporary variables
    std::string str;

    // Version::version
    this->version_ = this->ini->geti("Version", "version", -1);
    if (this->version_ < 0) {
        Log::writeError("[CONFIG] Failed to get .ini version");
    }

    // General::confirm_clear_queue
    this->confirmClearQueue_ = this->ini->getbool("General", "confirm_clear_queue");

    // General::confirm_exit
    this->confirmExit_ = this->ini->getbool("General", "confirm_exit");

    // General::initial_frame
    str = this->ini->gets("General", "initial_frame");
    if (str.empty()) {
        Log::writeError("[CONFIG] Failed to get initial_frame");
    }
    if (str == "Playlists") {
        this->initialFrame_ = Frame::Type::Playlists;
    } else if (str == "Albums") {
        this->initialFrame_ = Frame::Type::Albums;
    } else if (str == "Artists") {
        this->initialFrame_ = Frame::Type::Artists;
    } else {
        this->initialFrame_ = Frame::Type::Songs;
    }

    // General::log_level
    str = this->ini->gets("General", "log_level");
    if (str.empty()) {
        Log::writeError("[CONFIG] Failed to get log_level");
    }
    if (str == "None") {
        this->logLevel_ = Log::Level::None;
    } else if (str == "Success") {
        this->logLevel_ = Log::Level::Success;
    } else if (str == "Error") {
        this->logLevel_ = Log::Level::Error;
    } else if (str == "Info") {
        this->logLevel_ = Log::Level::Info;
    } else {
        this->logLevel_ = Log::Level::Warning;
    }

    // General::skip_with_lr
    this->skipWithLR_ = this->ini->getbool("General", "skip_with_lr");

    // Appearance::accent_colour
    str = this->ini->gets("Appearance", "accent_colour");
    if (str.empty()) {
        Log::writeError("[CONFIG] Failed to get (Appearance) accent_colour");
    }
    if (str == "Red") {
        this->accentColour_ = Theme::Colour::Red;
    } else if (str == "Orange") {
        this->accentColour_ = Theme::Colour::Orange;
    } else if (str == "Yellow") {
        this->accentColour_ = Theme::Colour::Yellow;
    } else if (str == "Green") {
        this->accentColour_ = Theme::Colour::Green;
    } else if (str == "Purple") {
        this->accentColour_ = Theme::Colour::Purple;
    } else if (str == "Pink") {
        this->accentColour_ = Theme::Colour::Pink;
    } else {
        this->accentColour_ = Theme::Colour::Blue;
    }

    // Appearance::auto_player_palette
    this->autoPlayerPalette_ = this->ini->getbool("Appearance", "auto_player_palette");

    // Appearance::language
    str = this->ini->gets("Appearance", "language");
    if (str.empty()) {
        Log::writeError("[CONFIG] Failed to get (Appearance) language");
    }
    if (str == "English") {
        this->language_ = Language::English;
    } else if (str == "ChineseSimplified") {
        this->language_ = Language::ChineseSimplified;
    } else if (str == "Japanese") {
        this->language_ = Language::Japanese;
    } else {
        this->language_ = Language::Default;
    }

    // Appearance::show_touch_controls
    this->showTouchControls_ = this->ini->getbool("Appearance", "show_touch_controls");

    // Metadata::scan_on_launch
    this->scanOnLaunch_ = this->ini->getbool("Metadata", "scan_on_launch");

    // Search::max_playlists
    this->searchMaxPlaylists_ = this->ini->geti("Search", "max_playlists", -42069);
    if (this->searchMaxPlaylists_ < -1) {
        Log::writeError("[CONFIG] Failed to get (Search) max_playlists");
        this->searchMaxPlaylists_ = -1;
    }

    // Search::max_artists
    this->searchMaxArtists_ = this->ini->geti("Search", "max_artists", -42069);
    if (this->searchMaxArtists_ < -1) {
        Log::writeError("[CONFIG] Failed to get (Search) max_artists");
        this->searchMaxArtists_ = -1;
    }

    // Search::max_albums
    this->searchMaxAlbums_ = this->ini->geti("Search", "max_albums", -42069);
    if (this->searchMaxAlbums_ < -1) {
        Log::writeError("[CONFIG] Failed to get (Search) max_albums");
        this->searchMaxAlbums_ = -1;
    }

    // Search::max_songs
    this->searchMaxSongs_ = this->ini->geti("Search", "max_songs", -42069);
    if (this->searchMaxSongs_ < -1) {
        Log::writeError("[CONFIG] Failed to get (Search) max_songs");
        this->searchMaxSongs_ = -1;
    }

    // Advanced::auto_launch_service
    this->autoLaunchService_ = this->ini->getbool("Advanced", "auto_launch_service");

    // Advanced::set_queue_max
    this->setQueueMax_ = this->ini->geti("Advanced", "set_queue_max", -42069);
    if (this->setQueueMax_ < -1) {
        Log::writeError("[CONFIG] Failed to get set_queue_max");
        this->setQueueMax_ = -1;
    }

    // Advanced::search_max_score
    this->searchMaxScore_ = this->ini->geti("Advanced", "search_max_score", -42069);
    if (this->searchMaxScore_ < 0) {
        Log::writeError("[CONFIG] Failed to get (Advanced) search_max_score");
        this->searchMaxScore_ = 130;
    }

    // Advanced::search_max_phrases
    this->searchMaxPhrases_ = this->ini->geti("Advanced", "search_max_phrases", -42069);
    if (this->searchMaxPhrases_ < 0) {
        Log::writeError("[CONFIG] Failed to get (Advanced) search_max_phrases");
        this->searchMaxPhrases_ = 8;
    }
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
    return this->version_;
}

bool Config::setVersion(const int v) {
    bool ok = this->ini->put("Version", "version", v);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set .ini version");
    } else {
        this->version_ = v;
    }
    return ok;
}

bool Config::confirmClearQueue() {
    return this->confirmClearQueue_;
}

bool Config::setConfirmClearQueue(const bool b) {
    bool ok = this->ini->put("General", "confirm_clear_queue", (b ? "Yes" : "No"));
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set confirm_clear_queue");
    } else {
        this->confirmClearQueue_ = b;
    }
    return ok;
}

bool Config::confirmExit() {
    return this->confirmExit_;
}

bool Config::setConfirmExit(const bool b) {
    bool ok = this->ini->put("General", "confirm_exit", (b ? "Yes" : "No"));
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set confirm_exit");
    } else {
        this->confirmExit_ = b;
    }
    return ok;
}

Frame::Type Config::initialFrame() {
    return this->initialFrame_;
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
    } else {
        this->initialFrame_ = t;
    }
    return ok;
}

Log::Level Config::logLevel() {
    return this->logLevel_;
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
    } else {
        this->logLevel_ = l;
    }
    return ok;
}

bool Config::skipWithLR() {
    return this->skipWithLR_;
}

bool Config::setSkipWithLR(bool b) {
    bool ok = this->ini->put("General", "skip_with_lr", (b ? "Yes" : "No"));
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set skip_with_lr");
    } else {
        this->skipWithLR_ = b;
    }
    return ok;
}

Theme::Colour Config::accentColour() {
    return this->accentColour_;
}

bool Config::setAccentColour(Theme::Colour c) {
    bool ok = this->ini->put("Appearance", "accent_colour", Theme::colourToString(c));
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Appearance) accent_colour");
    } else {
        this->accentColour_ = c;
    }
    return ok;
}

bool Config::autoPlayerPalette() {
    return this->autoPlayerPalette_;
}

bool Config::setAutoPlayerPalette(const bool b) {
    bool ok = this->ini->put("Appearance", "auto_player_palette", (b ? "Yes" : "No"));
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Appearance) auto_player_palette");
    } else {
        this->autoPlayerPalette_ = b;
    }
    return ok;
}

Language Config::language() {
    return this->language_;
}

bool Config::setLanguage(const Language l) {
    std::string str = "";
    switch (l) {
        case Language::Default:
        default:
            str = "Default";
            break;

        case Language::English:
            str = "English";
            break;

        case Language::ChineseSimplified:
            str = "ChineseSimplified";
            break;

        case Language::Japanese:
            str = "Japanese";
            break;
    }

    bool ok = this->ini->put("Appearance", "language", str);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Appearance) language");
    } else {
        this->language_ = l;
    }
    return ok;
}

bool Config::showTouchControls() {
    return this->showTouchControls_;
}

bool Config::setShowTouchControls(const bool b) {
    bool ok = this->ini->put("Appearance", "show_touch_controls", (b ? "Yes" : "No"));
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Appearance) show_touch_controls");
    } else {
        this->showTouchControls_ = b;
    }
    return ok;
}

bool Config::scanOnLaunch() {
    return this->scanOnLaunch_;
}

bool Config::setScanOnLaunch(const bool b) {
    bool ok = this->ini->put("Metadata", "scan_on_launch", (b ? "Yes" : "No"));
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Metadata) scan_on_launch");
    } else {
        this->scanOnLaunch_ = b;
    }
    return ok;
}

int Config::searchMaxPlaylists() {
    return this->searchMaxPlaylists_;
}

bool Config::setSearchMaxPlaylists(const int i) {
    bool ok = this->ini->put("Search", "max_playlists", i);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Search) max_playlists");
    } else {
        this->searchMaxPlaylists_ = i;
    }
    return ok;
}

int Config::searchMaxArtists() {
    return this->searchMaxArtists_;
}

bool Config::setSearchMaxArtists(const int i) {
    bool ok = this->ini->put("Search", "max_artists", i);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Search) max_artists");
    } else {
        this->searchMaxArtists_ = i;
    }
    return ok;
}

int Config::searchMaxAlbums() {
    return this->searchMaxAlbums_;
}

bool Config::setSearchMaxAlbums(const int i) {
    bool ok = this->ini->put("Search", "max_albums", i);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Search) max_albums");
    } else {
        this->searchMaxAlbums_ = i;
    }
    return ok;
}

int Config::searchMaxSongs() {
    return this->searchMaxSongs_;
}

bool Config::setSearchMaxSongs(const int i) {
    bool ok = this->ini->put("Search", "max_songs", i);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Search) max_songs");
    } else {
        this->searchMaxSongs_ = i;
    }
    return ok;
}

bool Config::autoLaunchService() {
    return this->autoLaunchService_;
}

bool Config::setAutoLaunchService(const bool b) {
    bool ok = this->ini->put("Advanced", "auto_launch_service", (b ? "Yes" : "No"));
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Advanced) auto_launch_service");
    } else {
        this->autoLaunchService_ = b;
    }
    return ok;
}

int Config::setQueueMax() {
    return this->setQueueMax_;
}

bool Config::setSetQueueMax(const int v) {
    bool ok = this->ini->put("Advanced", "set_queue_max", v);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Advanced) set_queue_max");
    } else {
        this->setQueueMax_ = v;
    }
    return ok;
}

int Config::searchMaxScore() {
    return this->searchMaxScore_;
}

bool Config::setSearchMaxScore(const int i) {
    bool ok = this->ini->put("Advanced", "search_max_score", i);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Advanced) search_max_score");
    } else {
        this->searchMaxScore_ = i;
    }
    return ok;
}

int Config::searchMaxPhrases() {
    return this->searchMaxPhrases_;
}

bool Config::setSearchMaxPhrases(const int i) {
    bool ok = this->ini->put("Advanced", "search_max_phrases", i);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Advanced) search_max_phrases");
    } else {
        this->searchMaxPhrases_ = i;
    }
    return ok;
}

bool Config::sysKeyComboEnabled() {
    if (!this->sysIni) {
        Log::writeError("[CONFIG] Can't access sysmodule config as object was not prepared");
        return false;
    }

    return this->sysIni->getbool("General", "key_combo_enabled");
}

bool Config::setSysKeyComboEnabled(const bool b) {
    if (!this->sysIni) {
        Log::writeError("[CONFIG] Can't access sysmodule config as object was not prepared");
        return false;
    }

    bool ok = this->sysIni->put("General", "key_combo_enabled", (b ? "Yes" : "No"));
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Sysmodule) key_combo_enabled");
    }
    return ok;
}

std::vector<NX::Button> Config::sysKeyComboNext() {
    if (!this->sysIni) {
        Log::writeError("[CONFIG] Can't access sysmodule config as object was not prepared");
        return NX::stringToCombo("L+DRIGHT+RSTICK");
    }

    std::string str = this->sysIni->gets("General", "key_combo_next");
    if (str.empty()) {
        Log::writeError("[CONFIG] Failed to get (Sysmodule) key_combo_next");
    }

    return NX::stringToCombo(str.empty() ? "L+DRIGHT+RSTICK" : str);
}

bool Config::setSysKeyComboNext(const std::vector<NX::Button> & combo) {
    if (!this->sysIni) {
        Log::writeError("[CONFIG] Can't access sysmodule config as object was not prepared");
        return false;
    }

    std::string str = NX::comboToString(combo);
    bool ok = this->sysIni->put("General", "key_combo_next", str);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Sysmodule) key_combo_next");
    }
    return ok;
}

std::vector<NX::Button> Config::sysKeyComboPlay() {
    if (!this->sysIni) {
        Log::writeError("[CONFIG] Can't access sysmodule config as object was not prepared");
        return NX::stringToCombo("L+DUP+RSTICK");
    }

    std::string str = this->sysIni->gets("General", "key_combo_play");
    if (str.empty()) {
        Log::writeError("[CONFIG] Failed to get (Sysmodule) key_combo_play");
    }

    return NX::stringToCombo(str.empty() ? "L+DUP+RSTICK" : str);
}

bool Config::setSysKeyComboPlay(const std::vector<NX::Button> & combo) {
    if (!this->sysIni) {
        Log::writeError("[CONFIG] Can't access sysmodule config as object was not prepared");
        return false;
    }

    std::string str = NX::comboToString(combo);
    bool ok = this->sysIni->put("General", "key_combo_play", str);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Sysmodule) key_combo_play");
    }
    return ok;
}

std::vector<NX::Button> Config::sysKeyComboPrev() {
    if (!this->sysIni) {
        Log::writeError("[CONFIG] Can't access sysmodule config as object was not prepared");
        return NX::stringToCombo("L+DLEFT+RSTICK");
    }

    std::string str = this->sysIni->gets("General", "key_combo_prev");
    if (str.empty()) {
        Log::writeError("[CONFIG] Failed to get (Sysmodule) key_combo_prev");
    }

    return NX::stringToCombo(str.empty() ? "L+DLEFT+RSTICK" : str);
}

bool Config::setSysKeyComboPrev(const std::vector<NX::Button> & combo) {
    if (!this->sysIni) {
        Log::writeError("[CONFIG] Can't access sysmodule config as object was not prepared");
        return false;
    }

    std::string str = NX::comboToString(combo);
    bool ok = this->sysIni->put("General", "key_combo_prev", str);
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Sysmodule) key_combo_prev");
    }
    return ok;
}

bool Config::sysPauseOnSleep() {
    if (!this->sysIni) {
        Log::writeError("[CONFIG] Can't access sysmodule config as object was not prepared");
        return false;
    }

    return this->sysIni->getbool("General", "pause_on_sleep");
}

bool Config::setSysPauseOnSleep(const bool b) {
    if (!this->sysIni) {
        Log::writeError("[CONFIG] Can't access sysmodule config as object was not prepared");
        return false;
    }

    bool ok = this->sysIni->put("General", "pause_on_sleep", (b ? "Yes" : "No"));
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Sysmodule) pause_on_sleep");
    }
    return ok;
}

bool Config::sysPauseOnUnplug() {
    if (!this->sysIni) {
        Log::writeError("[CONFIG] Can't access sysmodule config as object was not prepared");
        return false;
    }

    return this->sysIni->getbool("General", "pause_on_unplug");
}

bool Config::setSysPauseOnUnplug(const bool b) {
    if (!this->sysIni) {
        Log::writeError("[CONFIG] Can't access sysmodule config as object was not prepared");
        return false;
    }

    bool ok = this->sysIni->put("General", "pause_on_unplug", (b ? "Yes" : "No"));
    if (!ok) {
        Log::writeError("[CONFIG] Failed to set (Sysmodule) pause_on_unplug");
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
            eq[idx] = strtof(tok, nullptr);
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
