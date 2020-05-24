#ifndef THEME_HPP
#define THEME_HPP

#include "Aether.hpp"

namespace Main {
    // Stores colours for the current theme
    class Theme {
        private:
            Aether::Colour bottomBG_;
            Aether::Colour popupBG_;
            Aether::Colour sideBG_;

            Aether::Colour muted_;
            Aether::Colour muted2_;

            Aether::Colour FG_;

            Aether::Colour accent_;

            Aether::Colour selected_;

        public:
            // Sets colours (for now)
            Theme();

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
    };
};

#endif