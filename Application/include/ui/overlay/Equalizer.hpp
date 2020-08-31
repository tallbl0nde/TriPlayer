#ifndef OVERLAY_EQUALIZER_HPP
#define OVERLAY_EQUALIZER_HPP

#include <array>
#include "ui/element/VSlider.hpp"
#include "ui/overlay/Overlay.hpp"

namespace CustomOvl {
    class Equalizer : public Overlay {
        private:
            // Array of vertical sliders forming equalizer
            std::array<CustomElm::VSlider *, 32> sliders;

            std::string labelApply;       // Label for apply button
            std::string labelBack;        // Label for back button
            std::string labelOK;          // Label for OK button
            Aether::Controls * ctrl;      // Pointer to controls
            Aether::List * list;          // Pointer to list
            Aether::Rectangle * rect;     // Pointer to main rectangle
            Aether::Rectangle * top;      // Pointer to top rectangle
            Aether::Rectangle * bottom;   // Pointer to bottom rectangle
            Aether::Text * title;         // Title

            // Callback when apply is pressed
            std::function<void()> applyCallback;

        public:
            // Renders + positions elements
            Equalizer(const std::string &);

            // Set callback for apply button
            void setApplyCallback(std::function<void()>);

            // Return values of sliders in order
            std::array<float, 32> getValues();

            // Set values of sliders in order
            void setValues(const std::array<float, 32> &);

            // Set strings
            void setApplyLabel(const std::string &);
            void setBackLabel(const std::string &);
            void setOKLabel(const std::string &);

            // Set colours
            void setBackgroundColour(const Aether::Colour &);
            void setHeadingColour(const Aether::Colour &);
            void setLineColour(const Aether::Colour &);
            void setSliderBackgroundColour(const Aether::Colour &);
            void setSliderForegroundColour(const Aether::Colour &);
            void setSliderKnobColour(const Aether::Colour &);
    };
};

#endif