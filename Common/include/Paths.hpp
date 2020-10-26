// This file contains constant strings representing file locations/directories on the SD card
// or romfs. It does not account for every file/directory, just those that are used in multiple
// locations throughout the entire project.
#ifndef PATHS_HPP
#define PATHS_HPP

#include <string>

namespace Path {
    // Shared paths
    namespace Common {
        extern const std::string ConfigFolder;
        extern const std::string SwitchFolder;

        extern const std::string DatabaseFile;
        extern const std::string DatabaseBackupFile;
    };

    // Application specific paths
    namespace App {
        extern const std::string ConfigFile;
        extern const std::string LogFile;

        extern const std::string UpdateFolder;
        extern const std::string UpdateInfo;

        extern const std::string DefaultArtFile;
        extern const std::string DefaultArtistFile;

        extern const std::string ArtistImageFolder;
        extern const std::string AlbumImageFolder;
        extern const std::string PlaylistImageFolder;
    };

    // Sysmodule specific paths
    namespace Sys {
        extern const std::string ConfigFile;
        extern const std::string LogFile;
    };
};

#endif