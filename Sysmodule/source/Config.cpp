#include "Config.hpp"
#include <cstring>
#include "minIni.h"
#include "utils/FS.hpp"

// Include the converted .ini (stored in data section)
#include "sys_config_ini.h"

Config::Config(const std::string & path) {
    // Check the file exists
    this->validFile = false;
    if (!Utils::Fs::fileExists(path)) {
        // Copy if it doesn't (this is awkward as the .ini isn't in romfs)
        std::vector<unsigned char> buf;
        buf.resize(sys_config_ini_size, 0);
        std::memcpy(&buf[0], sys_config_ini, sys_config_ini_size);

        if (!Utils::Fs::writeFile(path, buf)) {
            Log::writeError("[CONFIG] Unable to copy template, bad things may happen");
        }
    }

    // Prepare minIni to read from this file
    this->ini = new minIni(path);
    this->validFile = true;
}

int Config::version() {
    int version = this->ini->geti("Version", "version", -1);
    if (version < 0) {
        Log::writeError("[CONFIG] Failed to get .ini version");
    }

    return version;
}

Log::Level Config::logLevel() {
    const std::string level = this->ini->gets("General", "log_level", "");
    if (level.empty()) {
        Log::writeError("[CONFIG] Failed to get log level");
    }

    if (level == "Info") {
        return Log::Level::Info;

    } else if (level == "Success") {
        return Log::Level::Success;

    } else if (level == "Error") {
        return Log::Level::Error;

    } else if (level == "None") {
        return Log::Level::None;
    }

    return Log::Level::Warning;
}

bool Config::MP3AccurateSeek() {
    return this->ini->getbool("MP3", "accurate_seek");
}

std::array<float, 32> Config::MP3Equalizer() {
    std::array<float, 32> eq;
    eq.fill(1.0f);
    size_t idx;

    // Iterate over each row
    std::string key;
    for (size_t row = 0; row < 4; row++) {
        // Get the value for the row
        key = "equalizer_" + std::to_string((row*8) + 1) + "_" + std::to_string((row+1) * 8);
        const std::string str = this->ini->gets("MP3", key, "");
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

Config::~Config() {
    delete this->ini;
}