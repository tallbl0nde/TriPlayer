#include "Protocol.hpp"

namespace Protocol {
    // Character used to separate strings (record separator)
    const char Delimiter = '\x1e';
    // Port to listen on
    const int Port = 3333;
    // Seconds to wait for reply before timing out
    const int Timeout = 5;
    // Protocol version
    const int Version = 2;
};