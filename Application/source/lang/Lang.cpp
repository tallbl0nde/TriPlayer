#include <fstream>
#include "lang/Lang.hpp"
#include "lang/Language.hpp"
#include "nlohmann/json.hpp"
#include <sstream>
#include "utils/FS.hpp"
#include "utils/NX.hpp"

namespace Utils::Lang {
    // JSON object which reads and caches strings from file
    static nlohmann::json en = nullptr;
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

            case Language::Spanish:
                return "Español";

            case Language::Japanese:
                return "日本語";

            case Language::ChineseTraditional:
                return "繁體中文";

            case Language::ChineseSimplified:
                return "简体中文";

            case Language::Korean:
                return "한국어";

            default:
                break;
        }

        return "?";
    }

    bool setLanguage(const Language l) {
        // Also set the fallback language to English here on first set
        if (en == nullptr) {
            std::ifstream in("romfs:/lang/en.json");
            en = nlohmann::json::parse(in);
        }

        std::string path = "";
        Language lang = l;
        if (l == Language::Default) {
            lang = Utils::NX::getSystemLanguage();
        }

        switch (lang) {
            case Language::Spanish:
                path = "romfs:/lang/es.json";
                break;

            case Language::Japanese:
                path = "romfs:/lang/jp.json";
                break;

            case Language::ChineseTraditional:
                path = "romfs:/lang/zh-HANT.json";
                break;

            case Language::ChineseSimplified:
                path = "romfs:/lang/zh-HANS.json";
                break;

            case Language::Korean:
                path = "romfs:/lang/ko.json";
                break;

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
        bool haveString = false;
        bool isFallback = false;
        nlohmann::json t = j;
        while (!haveString) {
            std::istringstream ss(key);
            std::string k;
            while (std::getline(ss, k, '.') && t != nullptr) {
                t = t[k];
            }

            // If the string is not present...
            if (t == nullptr || !t.is_string()) {
                // Check fallback json, otherwise return key
                if (!isFallback) {
                    t = en;
                    isFallback = true;
                } else {
                    return key;
                }

            } else {
                haveString = true;
            }
        }

        return t.get<std::string>();
    }
};
