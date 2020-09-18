// This file contains constant strings representing file locations/directories on the SD card
// or romfs. It does not account for every file/directory, just those that are used in multiple
// locations throughout the entire project.
#include "Paths.hpp"

namespace Path {
    namespace Common {
        const std::string ConfigFolder = "/config/TriPlayer/";
        const std::string SwitchFolder = "/switch/TriPlayer/";

        const std::string DatabaseFile = Common::SwitchFolder + "data.sqlite3";
        const std::string DatabaseBackupFile = Common::SwitchFolder + "data_old.sqlite3";
    };

    namespace App {
        const std::string ConfigFile = Common::ConfigFolder + "app_config.ini";
        const std::string LogFile = Common::SwitchFolder + "application.log";

        const std::string DefaultArtFile = "romfs:/misc/noalbum.png";
        const std::string DefaultArtistFile = "romfs:/misc/noartist.png";

        const std::string AlbumImageFolder = Common::SwitchFolder + "images/album/";
        const std::string ArtistImageFolder = Common::SwitchFolder + "images/artist/";
        const std::string PlaylistImageFolder = Common::SwitchFolder + "images/playlist/";
    };

    namespace Sys {
        const std::string ConfigFile = Common::ConfigFolder + "sys_config.ini";
        const std::string LogFile = Common::SwitchFolder + "sysmodule.log";
    };
};