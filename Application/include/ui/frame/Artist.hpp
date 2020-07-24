#ifndef FRAME_ARTIST_HPP
#define FRAME_ARTIST_HPP

#include "Types.hpp"
#include "ui/frame/Frame.hpp"
#include "ui/overlay/ItemMenu.hpp"

namespace Frame {
    class Artist : public Frame {
        private:
            // Menu shown when 'more' is pressed
            CustomOvl::Menu * artistMenu;
            CustomOvl::ItemMenu * albumMenu;

            // Functions to create menus
            void createArtistMenu(ArtistID);
            void createAlbumMenu(AlbumID);

        public:
            // The constructor takes the ID of the artist to show
            Artist(Main::Application *, ArtistID);

            // Delete created overlays
            ~Artist();
    };
};

#endif