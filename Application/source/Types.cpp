#include "Types.hpp"

AudioFormat audioFormatFromString(const std::string & str) {
    if (str == "FLAC") {
        return AudioFormat::FLAC;

    } else if (str == "MP3") {
        return AudioFormat::MP3;
    }

    // Return none by default
    return AudioFormat::None;
}

std::string audioFormatToString(const AudioFormat format) {
    switch (format) {
        case AudioFormat::FLAC:
            return "FLAC";

        case AudioFormat::MP3:
            return "MP3";

        default:
            return "None";
    }
}