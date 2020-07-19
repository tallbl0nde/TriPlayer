#ifndef ELEMENT_LISTFILE_HPP
#define ELEMENT_LISTFILE_HPP

#include "Aether/Aether.hpp"

namespace CustomElm {
    // This element is used in FileBrowser and represents an item
    // within a directory
    class ListFile : public Aether::Element {
        private:
            enum RenderingStatus {
                Waiting,        // Not rendering and no textures
                InProgress,     // Rendering but no textures
                Done            // Rendered and textures done
            };
            RenderingStatus isRendering;

            // Elements
            Aether::Image * icon;
            Aether::Text * name;

            // Positions elements
            void positionItems();

        public:
            // Constructor takes name, if directory and callback
            ListFile(std::string, bool, std::function<void()>);

            // Set colours
            void setIconColour(Aether::Colour);
            void setTextColour(Aether::Colour);

            void update(uint32_t);
            void setW(int);
    };
};

#endif