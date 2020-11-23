#ifndef METADATA_M3U_HPP
#define METADATA_M3U_HPP

#include <string>
#include <vector>

namespace Metadata::M3U {
    // Data structure used to pass/return values to/from a .m3u8 file
    struct Playlist {
        std::string name;                 // Name of playlist (can be empty)
        std::vector<std::string> paths;   // Paths to songs in playlist
    };

    // Parse the provided .m3u or .m3u8 file and return a list of strings
    // pointing to potential audio files in the playlist (relative), as
    // well as the playlist name if present
    bool parseFile(const std::string &, Playlist &);

    // Create a .m3u8 file at the specified location with the given
    // playlist name and relative song paths
    bool writeFile(const std::string &, const Playlist &);
};

#endif