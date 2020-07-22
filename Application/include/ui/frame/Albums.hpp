#ifndef FRAME_ALBUMS_HPP
#define FRAME_ALBUMS_HPP

#include "ui/frame/Frame.hpp"
#include "ui/overlay/menu/ArtistList.hpp"
#include "ui/overlay/menu/Album.hpp"

namespace Frame {
    class Albums : public Frame {
        private:
            // Menu displayed when the dots are pressed
            CustomOvl::Menu::ArtistList * artistsMenu;
            CustomOvl::Menu::Album * menu;

            // Helper functions to prepare menus
            void createArtistsMenu(AlbumID);
            void createMenu(AlbumID);

        public:
            // Constructor sets strings and forms list using database
            Albums(Main::Application *);

            // Delete menu if there is one
            ~Albums();
    };
};

#endif