#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <array>
#include "Log.hpp"
#include <string>

// Forward declaration as we only need the pointer here
class minIni;

// The config class interacts with the config file stored on
// the SD Card to read (not write) the configuration of the
// sysmodule. Each option can be queried through this object.
class Config {
    private:
        // minIni interacts with .ini file
        minIni * ini;

        // Is the given file path valid?
        bool validFile;

    public:
        // Takes path to config file
        // Creates if it doesn't exist
        Config(const std::string &);

        // Version of config (-1 on error)
        int version();

        // Pause when headset unplugged
        bool pauseOnUnplug();

        // Logging level (defaults to Warning)
        Log::Level logLevel();

        // Seek method for mpg123 (defaults to false)
        bool MP3AccurateSeek();
        // Equalizer values for mpg123 (all 1.0 by default)
        // Returns all bands in order
        std::array<float, 32> MP3Equalizer();

        // Deletes minIni object
        ~Config();
};

#endif