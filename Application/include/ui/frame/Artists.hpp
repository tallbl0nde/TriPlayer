#ifndef FRAME_ARTISTS_HPP
#define FRAME_ARTISTS_HPP

#include "db/Database.hpp"
#include "ui/frame/Frame.hpp"
#include "ui/overlay/ItemMenu.hpp"

// Forward declarations
namespace CustomElm {
    class ScrollableGrid;
};
namespace CustomOvl {
    class SortBy;
};

namespace Frame {
    class Artists : public Frame {
        private:
            // Grid of items
            CustomElm::ScrollableGrid * grid;

            // Menu displayed when the dots are pressed
            CustomOvl::ItemMenu * menu;

            // Sort by menu
            CustomOvl::SortBy * sortMenu;

            // Helper function to prepare menu
            void createMenu(ArtistID);

            // (Re)create main list
            void createList(Database::SortBy);

        public:
            // Constructor sets strings and forms list using database
            Artists(Main::Application *);

            // Delete menu if there is one
            ~Artists();
    };
};

#endif