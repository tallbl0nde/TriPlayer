#ifndef ELEMENT_PLAYER_HPP
#define ELEMENT_PLAYER_HPP

#include "Slider.hpp"
#include "Types.hpp"

namespace CustomElm {
    // The PLayer element is used for the player controls, etc
    // at the bottom of the screen.
    class Player : public Aether::Container {
        private:
            Aether::Rectangle * playerBg;

            // Album art and track metadata
            Aether::Image * albumCover;
            Aether::Image * albumCoverDefault;
            Aether::Text * trackName;
            Aether::Text * trackArtist;
            Aether::Text * trackArtistDots;

            // Controls
            Aether::Image * shuffle;
            Aether::Image * previous;
            Aether::Image * play;
            Aether::Image * pause;
            Aether::Image * next;
            Aether::Image * repeat;
            Aether::Image * repeatOne;

            // Seekbar
            Aether::Text * position;
            CustomElm::Slider * seekBar;
            Aether::Text * duration;

            // Volume slider + fullscreen
            Aether::Image * volumeIcon;
            CustomElm::Slider * volume;
            Aether::Image * fullscreen;

            // Copy of variables used for updates
            Aether::Colour accent;
            Aether::Colour muted;
            unsigned int durationVal;
            std::function<void(RepeatMode)> repeatFunc;

            // Actually alters text and seek bar
            void setPosition_(double);

        public:
            // Prepares all elements (position + dimensions are hardcoded)
            Player();

            // Set element values (some will only take effect if not selected)
            void setAlbumCover(unsigned char *, size_t);
            void setTrackName(std::string);
            void setTrackArtist(std::string);
            void setShuffle(bool);
            void setPlaying(bool);
            void setRepeat(RepeatMode);
            void setPosition(double);
            void setDuration(unsigned int);
            void setVolume(double);

            // Set colours
            void setAccentColour(Aether::Colour);
            void setBackgroundColour(Aether::Colour);
            void setForegroundColour(Aether::Colour);
            void setMutedColour(Aether::Colour);
            void setMuted2Colour(Aether::Colour);

            // Set callbacks for elements
            void setShuffleCallback(std::function<void()>);
            void setPreviousCallback(std::function<void()>);
            void setPauseCallback(std::function<void()>);
            void setPlayCallback(std::function<void()>);
            void setNextCallback(std::function<void()>);
            void setRepeatCallback(std::function<void(RepeatMode)>);
            void setSeekCallback(std::function<void(float)>);
            void setVolumeIconCallback(std::function<void()>);
            void setVolumeCallback(std::function<void(float)>);
            void setFullscreenCallback(std::function<void()>);

            // Update things
            void update(uint32_t);
    };
};

#endif