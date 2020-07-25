#ifndef UTILS_METADATA_AUDIODB_HPP
#define UTILS_METADATA_AUDIODB_HPP

#include <string>
#include <vector>

namespace Metadata::AudioDB {
    struct Entry {
        int tadbID;                 // The AudioDB ID (-1 on parse error, -2 on download error)
        std::string name;           // Name
        std::string imageURL;       // URL to download image
    };

    // Returns AudioDB info of the album matching the provided name (ID negative if not found/error)
    Entry getAlbumInfo(const std::string &);

    // Returns AudioDB info of the artist matching the provided name (ID negative if not found/error)
    Entry getArtistInfo(const std::string &);

    // Returns image as binary data in vector (empty if error or not found)
    // Accepts Entry type (with imageURL filled in)
    std::vector<unsigned char> getEntryImage(const Entry &);
};

#endif