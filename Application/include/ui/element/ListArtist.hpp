#ifndef ELEMENT_LISTSONG_HPP
#define ELEMENT_LISTSONG_HPP

#include "Aether/Aether.hpp"

namespace CustomElm {
    class ListArtist : public Aether::Element {
        enum RenderingStatus {
            Waiting,        // Not rendering and no textures
            InProgress,     // Rendering but no textures
            Done            // Rendered and textures done
        };

        private:
            // Artist image
            Aether::Image * image;

            // Texts
            Aether::Text * name;
            Aether::Text * counts;
            Aether::Image * dots;

            // Callback when dots pressed
            std::function<void()> moreCallback;
            bool callMore;

            // Status of threaded elements
            RenderingStatus isRendering;

            // Positions items after rendering
            void positionItems();

        public:
            // Constructor sets up elements and takes path to image
            ListArtist(int, int, std::string);

            // Override setInactive to deactivate touch on dots
            void setInactive();

            // Handle button press and pressing more button first
            bool handleEvent(Aether::InputEvent *);

            // Unhide once all elements have rendered
            void update(uint32_t);

            // Set callback for "more" button
            void setMoreCallback(std::function<void()>);

            // Set strings
            void setNameString(std::string);
            void setCountsString(std::string);

            // Set colours
            void setDotsColour(Aether::Colour);
            void setTextColour(Aether::Colour);
            void setMutedTextColour(Aether::Colour);
    };
};

#endif