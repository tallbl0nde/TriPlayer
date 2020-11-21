#ifndef UTILS_METADATA_HPP
#define UTILS_METADATA_HPP

#include <string>
#include "Types.hpp"
#include <vector>

namespace Metadata {
    // Result of download attempt
    enum class DownloadResult {
        Error,      // An unknown error occurred (likely connection/API?)
        NotFound,   // The artist was not found
        NoImage,    // There is no image associated with the artist
        Success     // Artist and image found/downloaded
    };

    // Searches for an album image given the album's name
    // Accepts name, buffer to fill with image, int to fill with ID
    DownloadResult downloadAlbumImage(const std::string &, std::vector<unsigned char> &, int &);

    // Searches for an artist image given the artist's name
    // Accepts name, buffer to fill with image, int to fill with ID
    DownloadResult downloadArtistImage(const std::string &, std::vector<unsigned char> &, int &);

    // Extracts appropriate album art from the given file
    // Returns an empty vector if no art is found
    std::vector<unsigned char> readArtFromFile(const std::string &, const AudioFormat);

    // Extracts metadata from the specified file
    // Returned ID is -1 on success, -2 on success but not enough tags, -3 on fatal error
    Song readFromFile(const std::string &, const AudioFormat);
};

#endif