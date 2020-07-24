#ifndef OVERLAY_ARTISTLIST_HPP
#define OVERLAY_ARTISTLIST_HPP

#include "ui/element/ListArtist.hpp"

namespace CustomOvl {
    // Overlay shown when 'View Artists' is selected
    class ArtistList : public Aether::Overlay {
        private:
            // Elements
            Aether::Rectangle * bg;
            Aether::List * list;
            bool touchOutside;

        public:
            // Renders + positions elements
            ArtistList();

            // Close if tapped outside
            bool handleEvent(Aether::InputEvent *);

            // Set rectangle colour
            void setBackgroundColour(Aether::Colour);

            // Add an entry to the list
            void addArtist(CustomElm::ListArtist *);
    };
};

#endif