#ifndef UTILS_METADATA_AUDIODB_HPP
#define UTILS_METADATA_AUDIODB_HPP

#include <string>
#include <vector>

namespace Metadata::AudioDB {
    // Returns image as binary data in vector (empty if error or not found)
    // Accepts MusicBrainz ID of artist
    std::vector<unsigned char> getArtistImage(const std::string &);
};

#endif