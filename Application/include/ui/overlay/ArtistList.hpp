#ifndef OVERLAY_ARTISTLIST_HPP
#define OVERLAY_ARTISTLIST_HPP

#include "ui/element/ListArtist.hpp"
#include "ui/overlay/Overlay.hpp"

namespace CustomOvl {
    // Overlay shown when 'View Artists' is selected
    class ArtistList : public Overlay {
        private:
            // Elements
            Aether::Rectangle * bg;
            Aether::List * list;

        public:
            // Renders + positions elements
            ArtistList();

            // Set rectangle colour
            void setBackgroundColour(Aether::Colour);

            // Add an entry to the list
            void addArtist(CustomElm::ListArtist *);
    };
};

#endif