#ifndef UTILS_SPLASH_HPP
#define UTILS_SPLASH_HPP

#include "Aether/Aether.hpp"

// These helper functions use the 'splash' library to return prominent colours
// in the given image
namespace Utils::Splash {
    // Struct to wrap returned values
    struct Palette {
        bool invalid;                   // Set true if an error occurred fetching the values
        bool bgLight;                   // Is the background colour light?
        Aether::Colour background;      // Colour to use for background
        Aether::Colour primary;         // Colour to use for primary text
        Aether::Colour secondary;       // Colour to use for secondary text
    };

    // Returns the above struct filled with colours for the given image
    Palette getPaletteForSurface(SDL_Surface *);

    // Change the given colour lightness by the specified amount
    Aether::Colour changeLightness(Aether::Colour, int);
};

#endif