#ifndef FRAME_ALBUM_HPP
#define FRAME_ALBUM_HPP

#include "Types.hpp"
#include "ui/frame/Frame.hpp"
#include "ui/overlay/ArtistList.hpp"
#include "ui/overlay/ItemMenu.hpp"

namespace Frame {
    class Album : public Frame {
        private:
            // Menu shown when 'more' is pressed
            CustomOvl::Menu * albumMenu;
            CustomOvl::ArtistList * artistsList;
            CustomOvl::ItemMenu * songMenu;

            // Functions to create menus
            void createAlbumMenu(AlbumID);
            void createArtistsList(AlbumID);
            void createSongMenu(SongID);

            // Helper function to play album
            void playAlbum(Metadata::Album);

        public:
            // The constructor takes the ID of the artist to show
            Album(Main::Application *, AlbumID);

            // Delete created overlays
            ~Album();
    };
};

#endif