#ifndef UTILS_METADATA_MUSICBRAINZ_HPP
#define UTILS_METADATA_MUSICBRAINZ_HPP

#include <string>
#include <vector>

namespace Metadata::MusicBrainz {
    struct Artist {
        std::string id;         // MusicBrainz Artist ID
        std::string name;       // Artist's name
        unsigned short score;   // Score representing how well they match (0 - 100)
    };

    // Returns a list of artists returned for the query, limited by second parameter
    std::vector<Artist> searchForArtists(const std::string &, unsigned int);
};

#endif