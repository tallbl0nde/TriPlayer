#ifndef OVERLAY_PROGRESSBOX_HPP
#define OVERLAY_PROGRESSBOX_HPP

#include "ui/overlay/Overlay.hpp"

namespace CustomOvl {
    // The ProgressBox overlay shows a progress bar
    // with percentage as well as a heading + subheading
    class ProgressBox : public Overlay {
        private:
            // Copy of elements in animation
            std::array<Aether::Image *, 50> animFrames;

            Aether::Rectangle * bg;             // Background rectangle
            Aether::Text * heading;             // Heading
            Aether::Text * subheading;          // Subheading
            Aether::Animation * anim;           // 'Spin' animation
            Aether::RoundProgressBar * pbar;    // Progress bar
            Aether::Text * perc;                // Percentage next to bar

        public:
            // Constructor accepts heading text
            ProgressBox();

            // Set strings
            void setHeadingText(const std::string &);
            void setSubheadingText(const std::string &);

            // Set value (updates bar + text)
            void setValue(const float);

            // Set colours
            void setBackgroundColour(const Aether::Colour &);
            void setTextColour(const Aether::Colour &);
            void setMutedTextColour(const Aether::Colour &);
            void setBarBackgroundColour(const Aether::Colour &);
            void setBarForegroundColour(const Aether::Colour &);
    };
};

#endif