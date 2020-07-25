#ifndef UTILS_METADATA_HPP
#define UTILS_METADATA_HPP

#include <string>
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
};

#endif