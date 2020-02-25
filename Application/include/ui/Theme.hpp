#ifndef THEME_HPP
#define THEME_HPP

#include "Aether.hpp"

namespace Main {
    // Stores colours for the current theme
    class Theme {
        private:
            Aether::Colour BG_;
            Aether::Colour sideBG_;
            Aether::Colour bottomBG_;

            Aether::Colour text_;
            Aether::Colour heading_;
            Aether::Colour mutedText_;

            Aether::Colour mutedLine_;

            Aether::Colour accent_;

            Aether::Colour selected_;

        public:
            // Sets colours (for now)
            Theme();

            // Returns private members
            Aether::Colour BG();
            Aether::Colour sideBG();
            Aether::Colour bottomBG();
            Aether::Colour text();
            Aether::Colour heading();
            Aether::Colour mutedText();
            Aether::Colour mutedLine();
            Aether::Colour accent();
            Aether::Colour selected();
    };
};

#endif