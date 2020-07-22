#ifndef ELEMENT_LISTARTIST_HPP
#define ELEMENT_LISTARTIST_HPP

#include "Aether/Aether.hpp"

namespace CustomElm {
    // A ListArtist appears in the ArtistList menu
    class ListArtist : public Aether::Element {
        enum RenderingStatus {
            Waiting,        // Not rendering and no textures
            InProgress,     // Rendering but no textures
            Done            // Rendered and textures done
        };

        private:
            // Elements
            Aether::Image * image;
            Aether::Text * name;

            // Rendering status
            RenderingStatus isRendering;

            // Positions elements
            void positionItems();

        public:
            // Constructor accepts path to image
            ListArtist(const std::string &);

            // Scroll name when selected
            void update(uint32_t);

            // Set elements
            void setImage(Aether::Image *);
            void setName(const std::string &);

            // Set text colour
            void setTextColour(Aether::Colour);
    };
};

#endif