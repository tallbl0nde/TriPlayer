#include <ctime>
#include <iomanip>
#include <fstream>
#include "Log.hpp"
#include "nlohmann/json.hpp"
#include "Paths.hpp"
#include "Updater.hpp"
#include "utils/Curl.hpp"
#include "utils/FS.hpp"

// URL used to retrieve update data
#define GITHUB_API_URL "https://api.github.com/repos/tallbl0nde/TriPlayer/releases/latest"

Updater::Updater() {
    this->downloadUrl = "";
    this->meta.changelog = "No changelog found.";
    this->meta.size = 0;
    this->meta.version = VER_STRING;

    // Create metadata file if it doesn't exist
    if (!Utils::Fs::fileExists(Path::App::UpdateInfo)) {
        Log::writeInfo("[UPDATE] Creating blank metadata file");
        std::vector<unsigned char> tmp = {'{' ,'\n', '}', '\n'};
        Utils::Fs::writeFile(Path::App::UpdateInfo, tmp);
    }
}

bool Updater::availableUpdate() {
    std::ifstream file(Path::App::UpdateInfo);
    nlohmann::json meta = nlohmann::json::parse(file);
    if (meta == nullptr) {
        Log::writeError("[UPDATE] [availableUpdate] Couldn't parse file");
        return false;
    }

    if (meta["version"] != nullptr) {
        return (meta["version"].get<std::string>() != "v" VER_STRING);
    }
    Log::writeInfo("[UPDATE] [availableUpdate] Couldn't find 'version' field");
    return false;
}

bool Updater::checkForUpdate() {
    // Make request to GitHub
    std::string response = "";
    bool success = Utils::Curl::downloadToString(GITHUB_API_URL, response);
    if (!success || response.length() == 0) {
        Log::writeError("[UPDATE] [checkForUpdate] Couldn't communicate with GitHub to get information");
        return false;
    }

    // Parse received JSON
    nlohmann::json j = nlohmann::json::parse(response);
    if (j == nullptr) {
        Log::writeError("[UPDATE] [checkForUpdate] Failed to parse JSON");
        return false;
    }

    // Extract relevant information
    bool ok = false;
    if (j["body"] != nullptr) {
        this->meta.changelog = j["body"].get<std::string>();
    }
    if (j["tag_name"] != nullptr) {
        this->meta.version = j["tag_name"].get<std::string>();
    }
    if (j["assets"] != nullptr) {
        if (j["assets"][0] != nullptr) {
            if (j["assets"][0]["size"] != nullptr) {
                this->meta.size = j["assets"][0]["size"].get<size_t>();
            }
            if (j["assets"][0]["browser_download_url"] != nullptr) {
                ok = true;
                this->downloadUrl = j["assets"][0]["browser_download_url"].get<std::string>();
            }
        }
    }

    // Update local metadata if we checked
    std::ifstream infile(Path::App::UpdateInfo);
    nlohmann::json meta = nlohmann::json::parse(infile);
    meta["lastCheck"] = std::time(nullptr);
    meta["version"] = this->meta.version;
    infile.close();

    std::ofstream outfile(Path::App::UpdateInfo);
    outfile << std::setw(4) << meta << std::endl;

    // Compare version to determine if it's an update or not
    if (ok && this->meta.version != "v" VER_STRING) {
        Log::writeSuccess("[UPDATE] [checkForUpdate] Update available: " + this->meta.version);
        return true;
    }

    Log::writeInfo("[UPDATE] [checkForUpdate] No update available");
    return false;
}

Updater::Meta Updater::getMetadata() {
    return this->meta;
}

bool Updater::needsCheck(const size_t range) {
    // Read metadata file into memory
    std::ifstream file(Path::App::UpdateInfo);
    nlohmann::json j = nlohmann::json::parse(file);
    if (j == nullptr) {
        Log::writeError("[UPDATE] [needsCheck] Couldn't read metadata file, assuming we need to check");
        return true;
    }

    // Return true if field isn't found
    if (j["lastCheck"] == nullptr) {
        Log::writeInfo("[UPDATE] [needsCheck] Couldn't find 'lastCheck' field");
        return true;
    }

    // Compare if timestamp is within range
    std::time_t now = std::time(nullptr);
    std::time_t last = j["lastCheck"].get<std::time_t>();
    if (last > now) {
        Log::writeWarning("[UPDATE] [needsCheck] Last checked time is in the future!");
        return false;
    }
    return (static_cast<size_t>(now - last) >= range);
}

bool Updater::downloadUpdate(std::function<void(long long, long long)> callback) {
    if (this->downloadUrl.empty()) {
        Log::writeError("[UPDATE] [downloadUpdate] Can't download as no url has been retrieved!");
        return false;
    }

    return Utils::Curl::downloadToFile(this->downloadUrl, Path::App::UpdateFile, callback);
}