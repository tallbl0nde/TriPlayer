#ifndef AETHER_THEME_HPP
#define AETHER_THEME_HPP

#include "Types.hpp"

// Animation speed factor for highlighting
#define ANIM_SPEED 1.8
// Pi :D
#define PI 3.14159265

// These are a set of colours provided that match what is used
// within Horizon.
namespace Aether::Theme {
    // Dark theme (Basic Black)
    const Theme_T Dark = {
        Colour{0, 255, 203, 255},   // accent
        Colour{49, 49, 49, 255},    // altBG
        Colour{45, 45, 45, 255},    // BG
        Colour{164, 164, 164, 255}, // FG
        Colour{35, 35, 40, 255},    // highlightBG
        Colour{100, 100, 100, 255}, // mutedLine
        Colour{160, 160, 160, 255}, // mutedText
        Colour{0, 250, 200, 50},    // selected
        Colour{255, 255, 255, 255}, // text
        [](uint32_t t){             // highlightFunc
            Colour col = {0, 0, 0, 255};
            col.g = 200 + (50 * sin(ANIM_SPEED * (t/1000.0) * PI));
            col.b = 220 + (30 * sin(ANIM_SPEED * (t/1000.0) * PI));
            return col;
        }
    };

    // Light theme (Basic White)
    const Theme_T Light = {
        Colour{50, 80, 240, 255},   // accent
        Colour{231, 231, 231, 255}, // altBG
        Colour{235, 235, 235, 255}, // BG
        Colour{45, 45, 45, 255},    // FG
        Colour{253, 253, 253, 255}, // highlightBG
        Colour{200, 200, 200, 255}, // mutedLine
        Colour{130, 130, 130, 255}, // mutedText
        Colour{0, 250, 200, 50},    // selected
        Colour{0, 0, 0, 255},       // text
        [](uint32_t t){             // highlightFunc
            Colour col = {0, 0, 0, 255};
            col.g = 225 + (25 * sin(ANIM_SPEED * (t/1000.0) * PI));
            col.b = 203 + (13 * sin(ANIM_SPEED * (t/1000.0) * PI));
            return col;
        }
    };
};

#endif