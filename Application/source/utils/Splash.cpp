#include "Log.hpp"
#include "splash/Splash.hpp"
#include "utils/Splash.hpp"

namespace Utils::Splash {
    Aether::Colour changeLightness(Aether::Colour old, int val) {
        ::Splash::Colour col = ::Splash::Colour(old.a(), old.r(), old.g(), old.b());
        col = ::Splash::ColourUtils::changeColourLightness(col, val);
        return Aether::Colour{(uint8_t)col.r(), (uint8_t)col.g(), (uint8_t)col.b(), (uint8_t)col.a()};
    }

    Palette getPaletteForDrawable(Aether::Drawable * drawable) {
        // Struct to return
        Palette palette;
        palette.invalid = true;

        // Check we actually have a surface
        if (drawable == nullptr || drawable->type() == Aether::Drawable::Type::None) {
            Log::writeError("[SPLASH] Attempted to get a palette for an invalid drawable");
            return palette;
        }

        // Get pixels from the drawable
        Aether::ImageData imageData = drawable->getImageData();
        std::vector<Aether::Colour> colours = imageData.toColourVector();
        if (!imageData.valid() || colours.size() == 0) {
            Log::writeError("[SPLASH] Attempted to get a palette for an invalid drawable");
            return palette;
        }

        // Convert to Splash::Colour
        std::vector<::Splash::Colour> pixels;
        for (const Aether::Colour & c : colours) {
            pixels.push_back(::Splash::Colour(c.a(), c.r(), c.g(), c.b()));
        }

        // Now create a Splash::Bitmap and fill with pixels
        ::Splash::Bitmap image = ::Splash::Bitmap(drawable->width(), drawable->height());
        size_t amt = image.setPixels(pixels, 0, 0, image.getWidth(), image.getHeight());
        if (amt != pixels.size()) {
            Log::writeWarning("[SPLASH] Not enough pixels were written to the bitmap - this may result in incorrect colours being picked (wrote: " + std::to_string(amt) + ", wanted: " + std::to_string(drawable->width() * drawable->height()) + ")");
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

    Aether::Colour interpolateColours(const Aether::Colour & start, const Aether::Colour & end, double amt) {
        amt = (amt < 0.0 ? 0.0 : amt);
        amt = (amt > 1.0 ? 1.0 : amt);

        double r = ((1.0 - amt) * start.r()) + (amt * end.r());
        double g = ((1.0 - amt) * start.g()) + (amt * end.g());
        double b = ((1.0 - amt) * start.b()) + (amt * end.b());
        double a = ((1.0 - amt) * start.a()) + (amt * end.a());
        return Aether::Colour(r, g, b, a);
    }
};
