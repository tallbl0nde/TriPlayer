#include "utils/metadata/AudioDB.hpp"
#include "utils/metadata/Metadata.hpp"

namespace Metadata {
    DownloadResult downloadAlbumImage(const std::string & name, std::vector<unsigned char> & data, int & id) {
        // First search for album
        AudioDB::Entry entry = Metadata::AudioDB::getAlbumInfo(name);
        if (entry.tadbID > 0) {
            id = entry.tadbID;
            // Now check if we have a URL
            if (entry.imageURL.empty()) {
                return DownloadResult::NoImage;
            }

            // Download image
            data = Metadata::AudioDB::getEntryImage(entry);
            if (data.size() > 0) {
                return DownloadResult::Success;
            }

        } else if (entry.tadbID == -1) {
            // Parse error
            return DownloadResult::NotFound;
        }

        return DownloadResult::Error;
    }

    DownloadResult downloadArtistImage(const std::string & name, std::vector<unsigned char> & data, int & id) {
        // First search for artist
        AudioDB::Entry entry = Metadata::AudioDB::getArtistInfo(name);
        if (entry.tadbID > 0) {
            id = entry.tadbID;
            // Now check if we have a URL
            if (entry.imageURL.empty()) {
                return DownloadResult::NoImage;
            }

            // Download image
            data = Metadata::AudioDB::getEntryImage(entry);
            if (data.size() > 0) {
                return DownloadResult::Success;
            }

        } else if (entry.tadbID == -1) {
            // Parse error
            return DownloadResult::NotFound;
        }

        return DownloadResult::Error;
    }
};