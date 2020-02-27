#ifndef AETHER_THREADED_TEXT_HPP
#define AETHER_THREADED_TEXT_HPP

#include "primary/Text.hpp"
#include "base/Threaded.hpp"

namespace Aether {
    // Same as text element except the rendering is done on a separate thread
    class ThreadedText : public Text, Threaded {
        private:
            // redrawTexture simply sets bool (to maintain compatibility with Text)
            void redrawTexture();
            // Whether to redraw texture
            bool redraw;

            // Function which actually redraws the texture as a _surface_
            void createSurface();
            // Surface which is returned
            SDL_Surface * surface;

        public:
            // Constructor accepts string, font size and font type
            ThreadedText(int, int, std::string, unsigned int, FontType = FontType::Normal, FontStyle = FontStyle::Regular);

            // Update handles redrawing on a thread
            void update(uint32_t);
    };
};

#endif