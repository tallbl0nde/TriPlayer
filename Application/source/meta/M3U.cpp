#include <fstream>
#include "Log.hpp"
#include "meta/M3U.hpp"
#include "utils/FS.hpp"

namespace Metadata::M3U {
    bool parseFile(const std::string & path, Playlist & playlist) {
        // Open file and check if stream is ok
        std::ifstream stream(path);
        if (!stream.good()) {
            Log::writeError("[META] [M3U] Failed to open " + path + " for reading");
            return false;
        }

        // Iterate over each line in the file
        for (std::string line; std::getline(stream, line);) {
            // Trim any leading whitespace
            line.erase(0, line.find_first_not_of(" \t"));

            // Skip over any comments
            if (line.length() > 0 && line[0] == '#') {
                // Grab the playlist's name if needed
                std::string key = line.substr(0, (line.length() < 10 ? line.length() : 10));
                if (key == "#PLAYLIST:") {
                    playlist.name = line.substr(10, line.length() - 10);
                    Log::writeInfo("[META] [M3U] Found playlist name: " + playlist.name);
                }

                continue;
            }

            // Ignore any paths that are URLs/URIs (also accounts for Windows drives)
            bool isPath = false;
            for (size_t i = 0; i < line.length(); i++) {
                // Check for ://, which indicates it's a URL/URI
                if (line.length() - i >= 3) {
                    if (line[i] == ':' && line[i+1] == '/' && line[i+2] == '/') {
                        isPath = true;
                        break;
                    }
                }

                // Check for :\ or :/, indicating it's an absolute Windows path
                if (line.length() - i >= 2) {
                    if (line[i] == ':' && (line[i+1] == '\\' || line[i+1] == '/')) {
                        isPath = true;
                        break;
                    }
                }
            }
            if (isPath) {
                continue;
            }

            // Otherwise append to playlist
            playlist.paths.push_back(line);
        }

        // Return successful if we made it this far
        Log::writeSuccess("[META] [M3U] Read " + std::to_string(playlist.paths.size()) + " song paths");
        return true;
    }

    bool writeFile(const std::string & path, const Playlist & playlist) {
        // Open an output stream at the specified location
        std::ofstream stream(path, std::ios::trunc);
        if (!stream.good()) {
            Log::writeError("[META] [M3U] Failed to create output file: " + path);
            return false;
        }

        // Write extended m3u header and playlist name
        stream << "#EXTM3U" << std::endl;
        if (!playlist.name.empty()) {
            stream << "#PLAYLIST:" << playlist.name << std::endl;
        }

        // Iterate over and print each path
        for (const std::string & path : playlist.paths) {
            stream << path << std::endl;
        }

        // Finally check if any errors occurred and if so delete the file
        stream.flush();
        if (!stream.good()) {
            Log::writeError("[META] [M3U] An error occurred writing to the file");
            stream.close();
            Utils::Fs::deleteFile(path);
            return false;
        }

        Log::writeSuccess("[META] [M3U] Wrote '" + playlist.name + " to disk");
        return true;
    }
};