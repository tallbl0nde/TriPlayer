#ifndef FRAME_ARTIST_HPP
#define FRAME_ARTIST_HPP

#include "db/Database.hpp"
#include "Types.hpp"
#include "ui/frame/Frame.hpp"
#include "ui/overlay/ItemMenu.hpp"

// Forward declaration
namespace CustomElm {
    class ScrollableGrid;
};
namespace CustomOvl {
    class SortBy;
};

namespace Frame {
    class Artist : public Frame {
        private:
            // Artist metadata
            Metadata::Artist meta;

            // Menu shown when 'more' is pressed
            CustomOvl::Menu * artistMenu;
            CustomOvl::ItemMenu * albumMenu;
            CustomOvl::SortBy * sortMenu;

            // Play button
            Aether::FilledButton * playButton;
            CustomElm::ScrollableGrid * grid;

            // Functions to create menus
            void createArtistMenu(ArtistID);
            void createAlbumMenu(AlbumID);
            void createList(Database::SortBy);

        public:
            // The constructor takes the ID of the artist to show
            Artist(Main::Application *, ArtistID);

            // Update colours
            void updateColours();

            // Delete created overlays
            ~Artist();
    };
};

#endif