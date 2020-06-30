#ifndef ELEMENT_PLAYER_HPP
#define ELEMENT_PLAYER_HPP

#include "RoundButton.hpp"
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
            CustomElm::RoundButton * shuffleC;
            Aether::Image * shuffle;
            CustomElm::RoundButton * previousC;
            Aether::Image * previous;
            CustomElm::RoundButton * playC;
            Aether::Image * play;
            CustomElm::RoundButton * pauseC;
            Aether::Image * pause;
            CustomElm::RoundButton * nextC;
            Aether::Image * next;
            CustomElm::RoundButton * repeatC;
            Aether::Image * repeat;
            CustomElm::RoundButton * repeatOneC;
            Aether::Image * repeatOne;

            // Seekbar
            Aether::Text * position;
            CustomElm::Slider * seekBar;
            Aether::Text * duration;

            // Volume slider + fullscreen
            CustomElm::RoundButton * volumeIconC;
            Aether::Image * volumeIcon;
            CustomElm::RoundButton * volumeIconMutedC;
            Aether::Image * volumeIconMuted;
            CustomElm::Slider * volume;
            CustomElm::RoundButton * fullscreenC;
            Aether::Image * fullscreen;

            // Copy of variables used for updates
            Aether::Colour accent;
            Aether::Colour muted;
            unsigned int durationVal;
            std::function<void(RepeatMode)> repeatFunc;
            std::function<void(float)> volumeFunc;

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
            void setVolumeIconCallback(std::function<void(bool)>);
            void setVolumeCallback(std::function<void(float)>);
            void setFullscreenCallback(std::function<void()>);

            // Update things
            void update(uint32_t);
    };
};

#endif