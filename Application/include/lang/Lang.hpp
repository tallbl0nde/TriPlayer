#ifndef UTILS_LANG_HPP
#define UTILS_LANG_HPP

#include <string>

// Forward declare enum (reduce compile time on additions)
enum class Language;

namespace Utils::Lang {
    // Return string representing language in native text
    std::string languageToString(const Language);

    // Set the language to use for future queries
    bool setLanguage(const Language);

    // Return the string matching given key
    // Defaults to English if current language does not have a string
    std::string string(const std::string &);
};

// Inline operator for string()
inline std::string operator ""_lang(const char * key, const size_t size) {
    return Utils::Lang::string(std::string(key, size));
}

#endif