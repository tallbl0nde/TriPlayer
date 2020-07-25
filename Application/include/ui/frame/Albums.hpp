#ifndef FRAME_ALBUMS_HPP
#define FRAME_ALBUMS_HPP

#include "ui/frame/Frame.hpp"
#include "ui/overlay/ArtistList.hpp"
#include "ui/overlay/ItemMenu.hpp"

namespace Frame {
    class Albums : public Frame {
        private:
            // Menu displayed when the dots are pressed
            CustomOvl::ArtistList * artistsList;
            CustomOvl::ItemMenu * albumMenu;

            // Helper functions to prepare menus
            void createArtistsList(AlbumID);
            void createMenu(AlbumID);

        public:
            // Constructor sets strings and forms list using database
            Albums(Main::Application *);

            // Delete menu if there is one
            ~Albums();
    };
};

#endif