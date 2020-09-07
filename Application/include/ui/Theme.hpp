#ifndef THEME_HPP
#define THEME_HPP

#include "Aether/Aether.hpp"

// Stores colours for the current theme
class Theme {
    public:
        // Accent colour presets
        enum class Colour {
            Red,
            Orange,
            Yellow,
            Green,
            Blue,
            Purple,
            Pink
        };

        // Return string equivalent of colour enum
        static std::string colourToString(const Colour);

    private:
        Aether::Colour bottomBG_;
        Aether::Colour popupBG_;
        Aether::Colour sideBG_;

        Aether::Colour muted_;
        Aether::Colour muted2_;

        Aether::Colour FG_;

        Colour accentColour;

        Aether::Colour selected_;

    public:
        // Sets colours (for now)
        Theme();

        // Set accent colour using enum
        void setAccent(Colour);

        // Returns private members
        Aether::Colour bottomBG();
        Aether::Colour popupBG();
        Aether::Colour sideBG();
        Aether::Colour muted();
        Aether::Colour muted2();
        Aether::Colour FG();
        Aether::Colour heading();
        Aether::Colour mutedText();
        Aether::Colour mutedLine();
        Aether::Colour accent();
        Aether::Colour selected();
        std::function<Aether::Colour(uint32_t)> highlightFunc();
};

#endif