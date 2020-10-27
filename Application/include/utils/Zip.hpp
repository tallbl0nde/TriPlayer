#ifndef UTILS_ZIP_HPP
#define UTILS_ZIP_HPP

#include <string>

namespace Utils::Zip {
    // Extract the contents of the passed zip file to the given destination,
    // overwriting any existing files
    bool extract(const std::string &, const std::string &);
}

#endif