#ifndef AETHER_BASEPROGRESS_HPP
#define AETHER_BASEPROGRESS_HPP

#include "base/Element.hpp"

namespace Aether {
    // Base element for progress bars that handles the value
    class BaseProgress : public Element {
        private:
            // Value of progress bar from 0.0 to 100.0
            // Any less/greater will be rounded down/up
            float value_;

            // Redraw texture(s)
            virtual void redrawBar() = 0;

        public:
            // X, Y, W, H
            BaseProgress(int, int, int, int);

            // Getter + setter for value
            float value();
            virtual void setValue(float);
    };
};

#endif