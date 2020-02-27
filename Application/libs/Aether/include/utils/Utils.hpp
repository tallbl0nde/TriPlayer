#ifndef AETHER_UTILS_HPP
#define AETHER_UTILS_HPP

#include "Types.hpp"

// Fake controller ID for held events
const short int FAKE_ID = 99;

namespace Aether::Utils {
    // Returns Button:: equivalent of value
    Button SDLtoButton(uint8_t);
};

#endif