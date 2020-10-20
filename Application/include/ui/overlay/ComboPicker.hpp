#ifndef OVERLAY_COMBOPICKER_HPP
#define OVERLAY_COMBOPICKER_HPP

#include "ui/element/ButtonPicker.hpp"
#include "ui/overlay/Overlay.hpp"

namespace CustomOvl {
    class ComboPicker : public Overlay {
        private:
            // Callback to invoke when combo confirmed
            std::function<void(std::vector<Aether::Button>)> callback;

            // Pair of elements forming one picker
            struct Picker {
                CustomElm::ButtonPicker * picker;
                Aether::BorderButton * remove;
            };

            // Vector of button pickers and remove buttons
            std::vector<Picker> pickers;

            // Additional elements
            Aether::Rectangle * rect;
            Aether::Text * title;
            Aether::Rectangle * topR;
            Aether::Rectangle * bottomR;
            Aether::Controls * ctrl;
            Aether::Text * tip;
            Aether::BorderButton * ok;

            // Button labels
            std::string labelBack;

        public:
            ComboPicker();

            // Set buttons and callback
            void setCombo(const std::vector<Aether::Button> &);
            void setCallback(const std::function<void(std::vector<Aether::Button>)> &);

            // Set colours
            void setBackgroundColour(const Aether::Colour &);
            void setButtonActiveColour(const Aether::Colour &);
            void setButtonInactiveColour(const Aether::Colour &);
            void setLineColour(const Aether::Colour &);
            void setMutedTextColour(const Aether::Colour &);
            void setTextColour(const Aether::Colour &);

            // Set labels
            void setBackLabel(const std::string &);
            void setOKLabel(const std::string &);
            void setRemoveLabel(const std::string &);

            void setTitleText(const std::string &);
            void setTipText(const std::string &);
    };
};

#endif