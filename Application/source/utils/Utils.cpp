#include <iostream>
#include "Utils.hpp"

namespace Utils {
    void writeStdout(std::string str) {
        #ifndef _NXLINK_
            return;
        #endif

        std::cout << str << std::endl;
    }
};