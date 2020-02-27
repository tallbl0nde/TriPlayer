#ifndef AETHER_ROUNDPROGRESSBAR_HPP
#define AETHER_ROUNDPROGRESSBAR_HPP

#include "base/BaseProgress.hpp"
#include "primary/Rectangle.hpp"

namespace Aether {
    // A RoundProgressBar is a completely filled progress bar rounded at either end.
    class RoundProgressBar : public BaseProgress {
        private:
            // "Greyed out" progress texture
            Colour bgColour;
            Rectangle * backTex;
            // Actual highlighted progress texture
            Colour fgColour;
            Rectangle * progressTex;

            // Rerender progressTex (on value change)
            void redrawBar();

        public:
            // X, Y, W, H (defaults to 12)
            RoundProgressBar(int, int, int, int = 12);

            // Whenever value is changed the progressTex must be updated
            void setValue(float);

            // Textures must be adjusted when dimensions are changed
            void setW(int);
            void setH(int);

            // Getter + setter for colours
            Colour getBackgroundColour();
            void setBackgroundColour(Colour);
            Colour getForegroundColour();
            void setForegroundColour(Colour);
    };
};

#endif