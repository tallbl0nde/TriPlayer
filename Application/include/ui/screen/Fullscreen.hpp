#ifndef SCREEN_FULLSCREEN_HPP
#define SCREEN_FULLSCREEN_HPP

#include "ui/element/Image.hpp"
#include "ui/element/RoundButton.hpp"
#include "ui/element/Slider.hpp"

namespace Main {
    class Application;
};

namespace Screen {
    // Fullscreen player
    class Fullscreen : public Aether::Screen {
        private:
            // Pointer to application object
            Main::Application * app;

            // Background
            Aether::Image * bg;
            Aether::Image * gradient;

            // Metadata
            std::vector<CustomElm::Image *> oldAlbumArt;
            CustomElm::Image * albumArt;
            Aether::Text * title;
            Aether::Text * artist;

            // Controls
            Aether::Container * controls;
            Aether::Image * shuffle;
            CustomElm::RoundButton * shuffleC;
            Aether::Image * previous;
            CustomElm::RoundButton * previousC;
            Aether::Image * play;
            CustomElm::RoundButton * playC;
            Aether::Image * pause;
            CustomElm::RoundButton * pauseC;
            Aether::Image * next;
            CustomElm::RoundButton * nextC;
            Aether::Container * repeatContainer;
            Aether::Image * repeat;
            CustomElm::RoundButton * repeatC;
            Aether::Container * repeatOneContainer;
            Aether::Image * repeatOne;
            CustomElm::RoundButton * repeatOneC;

            // Seek bar
            Aether::Text * position;
            CustomElm::Slider * seekBar;
            Aether::Text * duration;

            // Cache the song ID to avoid updating every frame
            SongID playingID;
            unsigned int durationVal;

            // Colours matching the album art
            double interpolatePos;
            Aether::Colour oldBackground;
            Aether::Colour currentBackground;
            Aether::Colour targetBackground;
            Aether::Colour primary;
            Aether::Colour secondary;
            Aether::Colour tertiary;

            // Milliseconds since last controller input (set negative in order to fade in)
            int buttonMs;

            // Returns the current colour to use for the highlight
            Aether::Colour highlightColour(uint32_t);

            // Set all element colours based on primary/secondary colours
            void setColours();

            // Updates the image and extracts/sets colours based on image
            void updateImage(const std::string &);

        public:
            Fullscreen(Main::Application *);

            // Observe events to determine when to fade the highlight
            bool handleEvent(Aether::InputEvent *);

            // Update player values
            void update(uint32_t);

            // onLoad creates all elements
            void onLoad();

            // onUnload deletes all elements
            void onUnload();
    };
};

#endif