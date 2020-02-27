#ifndef AETHER_PROGRESSBAR_HPP
#define AETHER_PROGRESSBAR_HPP

#include "base/BaseProgress.hpp"
#include "primary/Box.hpp"
#include "primary/Rectangle.hpp"

namespace Aether {
    // ProgressBar looks similar to Horizon's progress bar shown in data management
    class ProgressBar : public BaseProgress {
        private:
            // Border/outline
            Box * boxTex;
            // Actual progress bar
            Rectangle * progressTex;

            void redrawBar();

        public:
            ProgressBar(int, int, int);

            // When value is changed the mask of the rectangle must be changed
            void setValue(float);

            // Textures must be adjusted when dimensions are changed
            void setW(int);
            void setH(int);

            // Getter + setter for colour
            Colour getColour();
            void setColour(Colour);
    };
};

#endif