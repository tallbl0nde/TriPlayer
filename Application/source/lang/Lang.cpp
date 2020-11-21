#include <fstream>
#include "lang/Lang.hpp"
#include "lang/Language.hpp"
#include "nlohmann/json.hpp"
#include <sstream>
#include "utils/FS.hpp"
#include "utils/NX.hpp"

namespace Utils::Lang {
    // JSON object which reads and caches strings from file
    static nlohmann::json j = nullptr;

    // Read in a file, returning whether successful
    bool readFromFile(const std::string & path) {
        // Check file exists
        if (!Utils::Fs::fileExists(path)) {
            return false;
        }

        // Read in
        std::ifstream in(path);
        j = nlohmann::json::parse(in);
        return true;
    }

    std::string languageToString(const Language l) {
        switch (l) {
            case Language::Default:
                return "Common.DefaultLanguage"_lang;

            case Language::English:
                return "English";

            default:
                break;
        }

        return "?";
    }

    bool setLanguage(const Language l) {
        std::string path = "";

        Language lang;
        if (l == Language::Default) {
            lang = Utils::NX::getSystemLanguage();
        }

        switch (lang) {
            case Language::Default:
            case Language::English:
            default:
                path = "romfs:/lang/en.json";
                break;
        }

        return readFromFile(path);
    }

    std::string string(const std::string & key) {
        // First 'navigate' to nested object
        nlohmann::json t = j;
        std::istringstream ss(key);
        std::string k;
        while (std::getline(ss, k, '.') && t != nullptr) {
            t = t[k];
        }

        // If the string is not present return key
        if (t == nullptr || !t.is_string()) {
            return key;
        }

        return t.get<std::string>();
    }
};