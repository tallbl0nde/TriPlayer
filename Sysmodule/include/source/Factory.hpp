#ifndef SOURCE_FACTORY_HPP
#define SOURCE_FACTORY_HPP

#include "source/Source.hpp"
#include <string>

// Factory which creates an appropriate source based on the passed
// file's extension. It returns a nullptr if the file type is not
// supported.
namespace Source {
    class Factory {
        public:
            // Passed path of audio file
            static Source * getSource(const std::string &);
    };
};

#endif