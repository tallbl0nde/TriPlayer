#include "nlohmann/json.hpp"
#include "utils/Curl.hpp"
#include "utils/metadata/AudioDB.hpp"

// API key
#define API_KEY "1"
// Format of API request
#define ALBUM_API_REQUEST "https://theaudiodb.com/api/v1/json/" API_KEY "/searchalbum.php?a="
#define ARTIST_API_REQUEST "https://theaudiodb.com/api/v1/json/" API_KEY "/search.php?s="

namespace Metadata::AudioDB {
    Entry getAlbumInfo(const std::string & str) {
        Entry e;
        e.tadbID = -1;

        // Search TheAudioDB with the provided string
        std::string url = ALBUM_API_REQUEST + Utils::Curl::encodeString(str);
        std::string response = "";
        bool success = Utils::Curl::downloadToString(url, response);

        // Parse received JSON and retrieve important parts
        if (success && response.length() > 0) {
            nlohmann::json j = nlohmann::json::parse(response);
            if (j != nullptr) {
                if (j["album"] != nullptr) {
                    if (j["album"].size() > 0) {
                        if (j["album"][0]["idAlbum"] != nullptr) {
                            e.tadbID = std::stoi(j["album"][0]["idAlbum"].get<std::string>());
                        }
                        if (j["album"][0]["strAlbum"] != nullptr) {
                            e.name = j["album"][0]["strAlbum"].get<std::string>();
                        }
                        if (j["album"][0]["strAlbumThumb"] != nullptr) {
                            e.imageURL = j["album"][0]["strAlbumThumb"].get<std::string>();
                        }
                    }
                }
            }

        } else {
            e.tadbID = -2;
        }

        return e;
    }

    Entry getArtistInfo(const std::string & str) {
        Entry e;
        e.tadbID = -1;

        // Search TheAudioDB with the provided string
        std::string url = ARTIST_API_REQUEST + Utils::Curl::encodeString(str);
        std::string response = "";
        bool success = Utils::Curl::downloadToString(url, response);

        // Parse received JSON and retrieve important parts
        if (success && response.length() > 0) {
            nlohmann::json j = nlohmann::json::parse(response);
            if (j != nullptr) {
                if (j["artists"] != nullptr) {
                    if (j["artists"].size() > 0) {
                        if (j["artists"][0]["idArtist"] != nullptr) {
                            e.tadbID = std::stoi(j["artists"][0]["idArtist"].get<std::string>());
                        }
                        if (j["artists"][0]["strArtist"] != nullptr) {
                            e.name = j["artists"][0]["strArtist"].get<std::string>();
                        }
                        if (j["artists"][0]["strArtistThumb"] != nullptr) {
                            e.imageURL = j["artists"][0]["strArtistThumb"].get<std::string>();
                        }
                    }
                }
            }

        } else {
            e.tadbID = -2;
        }

        return e;
    }

    std::vector<unsigned char> getEntryImage(const Entry & e) {
        std::vector<unsigned char> v;

        // Download image from struct
        if (e.imageURL.length() > 0) {
            std::string url = e.imageURL + "/preview";
            bool success = Utils::Curl::downloadToBuffer(url, v);
            if (!success) {
                v.clear();
            }
        }

        return v;
    }
};