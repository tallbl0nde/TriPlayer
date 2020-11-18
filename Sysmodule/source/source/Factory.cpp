#include <cctype>
#include <filesystem>
#include "source/Factory.hpp"
#include "source/FLAC.hpp"
#include "source/MP3.hpp"

namespace Source {
    Source * Factory::getSource(const std::string & path) {
        // Get extension
        std::string ext = std::filesystem::path(path).extension();
        for (char & c : ext) {
            c = tolower(c);
        }

        // Match source based on extension
        if (ext == ".mp3") {
            return new MP3(path);

        } else if (ext == ".flac") {
            return new FLAC(path);
        }

        // No suitable class
        return nullptr;
    }
};