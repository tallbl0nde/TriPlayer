#ifndef UTILS_SEARCH_HPP
#define UTILS_SEARCH_HPP

#include <string>
#include <vector>

namespace Utils::Search {
    // Stores a string and it's score
    struct ScoredString {
        std::string string;     // String (can be word or phrase)
        int score;              // Score of the string (lower is better)
    };

    // Returns best ranking phrases given a vector of ordered (ascending) ScoredStrings
    // forming phrases where each 'string' is a word
    // Ordered best (lowest score) first
    std::vector<std::string> getPhrases(std::vector< std::vector<ScoredString> > &, size_t);
};

#endif