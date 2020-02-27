#ifndef AETHER_TYPES_HPP
#define AETHER_TYPES_HPP

#include <functional>
#include "SDLHelper.hpp"

namespace Aether {
    // Enums for buttons
    // Avoids confusion with SDL/libnx names
    enum Button {
        A,
        B,
        X,
        Y,
        LSTICK,
        RSTICK,
        L,
        R,
        ZL,
        ZR,
        PLUS,
        MINUS,
        DPAD_LEFT,
        DPAD_UP,
        DPAD_RIGHT,
        DPAD_DOWN,
        LSTICK_LEFT,
        LSTICK_UP,
        LSTICK_RIGHT,
        LSTICK_DOWN,
        RSTICK_LEFT,
        RSTICK_UP,
        RSTICK_RIGHT,
        RSTICK_DOWN,
        SL_LEFT,
        SR_LEFT,
        SL_RIGHT,
        SR_RIGHT,
        NO_BUTTON      // Dummy button used for symbolising no button
    };

    // SDL_Color but it's not
    typedef SDL_Color Colour;

    // Theme (used for provided themes)
    typedef struct {
        Colour accent;          // "Selected" colour
        Colour altBG;           // Lighter/darker shade of bg
        Colour bg;              // Background
        Colour fg;              // Foreground (ie. lines)
        Colour highlightBG;     // Highlight background (usually darker/lighter than bg)
        Colour mutedLine;       // Usually grey; used for lines that separate
        Colour mutedText;       // Also grey; used for less important text
        Colour selected;        // Colour to draw on top of element when selected
        Colour text;            // Text colour

        std::function<Colour(uint32_t)> highlightFunc;      // Function returning a colour (for highlight border) based on current time
    } Theme_T;
};

#endif