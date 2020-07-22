#ifndef OVERLAY_MENU_ARTISTLIST_HPP
#define OVERLAY_MENU_ARTISTLIST_HPP

#include "ui/overlay/menu/Menu.hpp"

// Forward declaration cause the include isn't necessary here
namespace CustomElm {
    class ListArtist;
};

namespace CustomOvl::Menu {
    // Overlay shown when 'View Artists' is selected
    class ArtistList : public Menu {
        private:
            // Elements
            Aether::List * list;

        public:
            // Renders + positions elements
            ArtistList();

            // Add an entry to the list
            void addArtist(CustomElm::ListArtist *);
    };
};

#endif