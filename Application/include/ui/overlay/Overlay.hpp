#ifndef OVERLAY_OVERLAY_HPP
#define OVERLAY_OVERLAY_HPP

#include "Aether/Aether.hpp"

namespace CustomOvl {
    // This overlay class extends the Aether::Overlay object to set a
    // darker background and allow it to be closed by pressing B or
    // touching the 'background'.
    class Overlay : public Aether::Overlay {
        private:
            // Pair of coordinates specifying top-left and bottom-right points of rectangle to
            // ignore touches in
            int x1, y1, x2, y2;

            // Set true when touch pressed outside of above rectangle
            bool touched;

        protected:
            // Set the above coordinates
            void setTopLeft(int, int);
            void setBottomRight(int, int);

        public:
            // Constructor creates additional background and sets up other things
            Overlay();

            // Check if touch event outside of marked rectangle
            bool handleEvent(Aether::InputEvent *);
    };
};

#endif