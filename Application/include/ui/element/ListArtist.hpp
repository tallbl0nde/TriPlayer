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
            // Static texture for top/bottom lines
            // This works as all ListArtist's will only be in the same list
            static SDL_Texture * lineTexture;
            Aether::Colour lineColour;

            // Texts
            Aether::Text * name;
            Aether::Text * counts;
            Aether::Image * dots;

            // Callback when dots pressed
            std::function<void()> moreCallback;
            bool callMore;

            // Status of threaded elements
            RenderingStatus isRendering;

            // Position items on x axis
            void positionItems();

        public:
            // Constructor sets up elements
            ListArtist();

            // Override setInactive to deactivate touch on dots
            void setInactive();

            // Handle button press and pressing more button first
            bool handleEvent(Aether::InputEvent *);

            // Position text once rendered
            void update(uint32_t);

            // Render lineTexture as well as normal rendering
            void render();

            // Set callback for "more" button
            void setMoreCallback(std::function<void()>);

            // Set strings
            void setNameString(std::string);
            void setCountsString(std::string);

            // Set colours
            void setDotsColour(Aether::Colour);
            void setLineColour(Aether::Colour);
            void setTextColour(Aether::Colour);
            void setMutedTextColour(Aether::Colour);

            // Reposition elements on width change
            void setW(int);
    };
};

#endif