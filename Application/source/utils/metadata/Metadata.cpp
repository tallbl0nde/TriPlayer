#include "utils/metadata/AudioDB.hpp"
#include "utils/metadata/Metadata.hpp"

namespace Metadata {
    DownloadResult downloadArtistImage(const std::string & name, std::vector<unsigned char> & data, int & id) {
        // First search for artist
        AudioDB::Artist artist = Metadata::AudioDB::getArtistInfo(name);
        if (artist.tadbID > 0) {
            id = artist.tadbID;
            // Now check if we have a URL
            if (artist.imageURL.empty()) {
                return DownloadResult::NoImage;
            }

            // Download image
            data = Metadata::AudioDB::getArtistImage(artist);
            if (data.size() > 0) {
                return DownloadResult::Success;
            }

        } else if (artist.tadbID == -1) {
            // Parse error
            return DownloadResult::NotFound;
        }

        return DownloadResult::Error;
    }
};