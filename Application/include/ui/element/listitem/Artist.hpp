#ifndef ELEMENT_LISTITEM_ARTIST_HPP
#define ELEMENT_LISTITEM_ARTIST_HPP

#include "ui/element/listitem/Item.hpp"

namespace CustomElm::ListItem {
    // An Artist appears in the ArtistList overlay
    class Artist : public Item {
        private:
            // Elements
            Aether::Image * image;
            Aether::Text * name;
            void positionElements();

        public:
            // Constructor accepts path to image
            Artist(const std::string &);

            // Scroll name when selected
            void update(uint32_t);

            // Set text things
            void setName(const std::string &);
            void setTextColour(Aether::Colour);
    };
};

#endif
