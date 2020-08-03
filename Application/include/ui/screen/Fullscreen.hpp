#ifndef SCREEN_FULLSCREEN_HPP
#define SCREEN_FULLSCREEN_HPP

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
            Aether::Image * albumArt;
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

            // Milliseconds since last controller input
            size_t buttonMs;

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