#include "nlohmann/json.hpp"
#include "Log.hpp"
#include "utils/Curl.hpp"
#include "meta/MusicBrainz.hpp"

// URL forming API request
#define API_REQUEST "http://musicbrainz.org/ws/2/artist/?fmt=json&query=artist:"

namespace Metadata::MusicBrainz {
    std::vector<Artist> searchForArtists(const std::string & name, unsigned int limit) {
        std::vector<Artist> v;

        // The url will be escaped/encoded by curl
        std::string url = API_REQUEST + Utils::Curl::encodeString(name);
        std::string response = "";
        bool success = Utils::Curl::downloadToString(url, response);
        if (!success || response.empty()) {
            return v;
        }

        // Convert to JSON object
        nlohmann::json j = nlohmann::json::parse(response);
        if (j != nullptr) {
            if (j["artists"] != nullptr) {
                // Iterate over each artist to populate an Artist struct
                for (size_t i = 0; i < j["artists"].size(); i++) {
                    if (v.size() >= limit) {
                        break;
                    }

                    Artist a;
                    if (j["artists"][i]["id"] != nullptr) {
                        a.id = j["artists"][i]["id"].get<std::string>();
                    }
                    if (j["artists"][i]["name"] != nullptr) {
                        a.name = j["artists"][i]["name"].get<std::string>();
                    }
                    if (j["artists"][i]["score"] != nullptr) {
                        a.score = j["artists"][i]["score"].get<unsigned short>();
                    }
                    v.push_back(a);
                }
            }
        }

        return v;
    }
};