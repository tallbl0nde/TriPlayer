#include <cctype>
#include "source/Factory.hpp"
#include "source/FLAC.hpp"
#include "source/MP3.hpp"
#include "source/WAV.hpp"
#include "utils/FS.hpp"

namespace Source {
    Source * Factory::getSource(const std::string & path) {
        // Get extension
        std::string ext = Utils::Fs::getExtension(path);
        for (char & c : ext) {
            c = tolower(c);
        }

        // Match source based on extension
        if (ext == ".mp3") {
            return new MP3(path);

        } else if (ext == ".flac") {
            return new FLAC(path);

        } else if (ext == ".wav" || ext == ".wave") {
            return new WAV(path);
        }

        // No suitable class
        return nullptr;
    }
};