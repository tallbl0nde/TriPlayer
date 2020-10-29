#ifndef FRAME_ALBUMS_HPP
#define FRAME_ALBUMS_HPP

#include "db/Database.hpp"
#include "ui/frame/Frame.hpp"
#include "ui/overlay/ArtistList.hpp"
#include "ui/overlay/ItemMenu.hpp"

// Forward declarations
namespace CustomElm {
    class ScrollableGrid;
};
namespace CustomOvl {
    class SortBy;
};

namespace Frame {
    class Albums : public Frame {
        private:
            // Grid of items
            CustomElm::ScrollableGrid * grid;

            // Menu displayed when the dots are pressed
            CustomOvl::ArtistList * artistsList;
            CustomOvl::ItemMenu * albumMenu;
            CustomOvl::SortBy * sortMenu;

            // Helper functions to prepare menus
            void createArtistsList(AlbumID);
            void createList(Database::SortBy);
            void createMenu(AlbumID);

        public:
            // Constructor sets strings and forms list using database
            Albums(Main::Application *);

            // Delete menu if there is one
            ~Albums();
    };
};

#endif