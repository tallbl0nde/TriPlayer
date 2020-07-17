#include "nlohmann/json.hpp"
#include "utils/Curl.hpp"
#include "utils/metadata/AudioDB.hpp"

// API key
#define API_KEY "1"
// Format of API request
#define API_REQUEST "https://theaudiodb.com/api/v1/json/" API_KEY "/search.php?s="

namespace Metadata::AudioDB {
    Artist getArtistInfo(const std::string & str) {
        Artist a;
        a.tadbID = -1;

        // Search TheAudioDB with the provided string
        std::string url = API_REQUEST + Utils::Curl::encodeString(str);
        std::string response = "";
        bool success = Utils::Curl::downloadToString(url, response);

        // Parse received JSON and retrieve important parts
        if (success && response.length() > 0) {
            nlohmann::json j = nlohmann::json::parse(response);
            if (j != nullptr) {
                if (j["artists"] != nullptr) {
                    if (j["artists"].size() > 0) {
                        if (j["artists"][0]["idArtist"] != nullptr) {
                            a.tadbID = std::stoi(j["artists"][0]["idArtist"].get<std::string>());
                        }
                        if (j["artists"][0]["strArtist"] != nullptr) {
                            a.name = j["artists"][0]["strArtist"].get<std::string>();
                        }
                        if (j["artists"][0]["strArtistThumb"] != nullptr) {
                            a.imageURL = j["artists"][0]["strArtistThumb"].get<std::string>();
                        }
                    }
                }
            }
        }

        return a;
    }

    std::vector<unsigned char> getArtistImage(const Artist & a) {
        std::vector<unsigned char> v;

        // Download image from struct
        if (a.imageURL.length() > 0) {
            std::string url = a.imageURL + "/preview";
            bool success = Utils::Curl::downloadToBuffer(url, v);
            if (!success) {
                v.clear();
            }
        }

        return v;
    }
};