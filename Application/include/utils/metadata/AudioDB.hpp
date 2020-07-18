#ifndef UTILS_METADATA_AUDIODB_HPP
#define UTILS_METADATA_AUDIODB_HPP

#include <string>
#include <vector>

namespace Metadata::AudioDB {
    struct Artist {
        int tadbID;                 // The AudioDB ID (-1 on parse error, -2 on download error)
        std::string name;           // Name
        std::string imageURL;       // URL to download artist image
    };

    // Returns AudioDB info of the artist matching the provided name (ID negative if not found/error)
    Artist getArtistInfo(const std::string &);

    // Returns image as binary data in vector (empty if error or not found)
    // Accepts Artist type (with imageURL filled in)
    std::vector<unsigned char> getArtistImage(const Artist &);
};

#endif