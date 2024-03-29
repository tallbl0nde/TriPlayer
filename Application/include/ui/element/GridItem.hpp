#ifndef ELEMENT_GRIDITEM_HPP
#define ELEMENT_GRIDITEM_HPP

#include "Aether/Aether.hpp"

namespace CustomElm {
    class GridItem : public Aether::AsyncItem {
        private:
            // Artist image
            Aether::Image * image;

            // Texts
            Aether::Text * main;
            Aether::Text * sub;
            Aether::Image * dots;

            // Callback when dots pressed
            std::function<void()> moreCallback;
            bool callMore;

            // Positions items after rendering
            void positionElements();

            // Helper to update text elements
            void processText(Aether::Text * &, std::function<Aether::Text * ()>);

        public:
            // Constructor sets up elements and takes path to image
            GridItem(std::string);

            // Override setInactive to deactivate touch on dots
            void setInactive();
            // Override setW to prevent list stretching out the element
            void setW(int);

            // Handle button press and pressing more button first
            bool handleEvent(Aether::InputEvent *);

            // Unhide once all elements have rendered
            void update(uint32_t);

            // Set callback for "more" button
            void setMoreCallback(std::function<void()>);

            // Set strings
            void setMainString(std::string);
            void setSubString(std::string);

            // Set colours
            void setDotsColour(Aether::Colour);
            void setTextColour(Aether::Colour);
            void setMutedTextColour(Aether::Colour);
    };
};

#endif
