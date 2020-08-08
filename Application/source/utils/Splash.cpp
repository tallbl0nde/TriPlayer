#include "Log.hpp"
#include "splash/Splash.hpp"
#include "utils/Splash.hpp"

namespace Utils::Splash {
    Palette getPaletteForSurface(SDL_Surface * surf) {
        // Struct to return
        Palette palette;
        palette.invalid = true;

        // Check we actually have a surface
        if (surf == nullptr) {
            Log::writeError("[SPLASH] Attempted to get a palette for a null surface");
            return palette;
        }

        // First read pixels from the surface and convert to Splash::Colour
        std::vector<::Splash::Colour> pixels;
        SDL_LockSurface(surf);
        size_t count = surf->w * surf->h;
        for (size_t i = 0; i < count; i++) {
            // Get RGB values
            uint8_t r, g, b, a;
            SDL_GetRGBA(*((uint32_t *)surf->pixels + i), surf->format, &r, &g, &b, &a);

            // Create object and push onto vector
            ::Splash::Colour colour = ::Splash::Colour(a, r, g, b);
            pixels.push_back(colour);
        }
        SDL_UnlockSurface(surf);

        // Now create a Splash::Bitmap and fill with pixels
        ::Splash::Bitmap image = ::Splash::Bitmap(surf->w, surf->h);
        size_t amt = image.setPixels(pixels, 0, 0, image.getWidth(), image.getHeight());
        if (amt != pixels.size()) {
            Log::writeWarning("[SPLASH] Not enough pixels were written to the bitmap - this may result in incorrect colours being picked (wrote: " + std::to_string(amt) + ", wanted: " + std::to_string(surf->w * surf->h) + ")");
        }
        pixels.clear();

        // Create a MediaStyle object to extract the colours
        ::Splash::MediaStyle style = ::Splash::MediaStyle(image);
        palette.invalid = false;
        palette.bgLight = style.isLight();

        ::Splash::Colour tmp = style.getBackgroundColour();
        palette.background = Aether::Colour{(uint8_t)tmp.r(), (uint8_t)tmp.g(), (uint8_t)tmp.b(), (uint8_t)tmp.a()};

        tmp = style.getPrimaryTextColour();
        palette.primary = Aether::Colour{(uint8_t)tmp.r(), (uint8_t)tmp.g(), (uint8_t)tmp.b(), (uint8_t)tmp.a()};

        tmp = style.getSecondaryTextColour();
        palette.secondary = Aether::Colour{(uint8_t)tmp.r(), (uint8_t)tmp.g(), (uint8_t)tmp.b(), (uint8_t)tmp.a()};

        return palette;
    }

    Aether::Colour changeLightness(Aether::Colour old, int val) {
        ::Splash::Colour col = ::Splash::Colour(old.a, old.r, old.g, old.b);
        col = ::Splash::ColourUtils::changeColourLightness(col, val);
        return Aether::Colour{(uint8_t)col.r(), (uint8_t)col.g(), (uint8_t)col.b(), (uint8_t)col.a()};
    }
};