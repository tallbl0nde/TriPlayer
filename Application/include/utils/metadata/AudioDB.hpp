#ifndef UTILS_METADATA_AUDIODB_HPP
#define UTILS_METADATA_AUDIODB_HPP

#include <string>
#include <vector>

namespace Metadata::AudioDB {
    struct Artist {
        int tadbID;                 // The AudioDB ID
        std::string name;           // Name
        std::string imageURL;       // URL to download artist image (preview size)
    };

    // Returns AudioDB info of the artist matching the provided name (ID negative if not found/error)
    Artist getArtistInfo(const std::string &);

    // Returns image as binary data in vector (empty if error or not found)
    // Accepts Artist type (with imageURL filled in)
    std::vector<unsigned char> getArtistImage(const Artist &);
};

#endif