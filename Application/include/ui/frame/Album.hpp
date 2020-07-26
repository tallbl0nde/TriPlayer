#ifndef FRAME_ALBUM_HPP
#define FRAME_ALBUM_HPP

#include "Types.hpp"
#include "ui/frame/Frame.hpp"

// Forward declarations cause only pointers are needed here
namespace CustomOvl {
    class ArtistList;
    class ItemMenu;
    class Menu;
};

namespace Frame {
    class Album : public Frame {
        private:
            // Menu shown when 'more' is pressed
            CustomOvl::Menu * albumMenu;
            CustomOvl::ArtistList * artistsList;
            CustomOvl::ItemMenu * songMenu;

            // Cached data
            Metadata::Album metadata;
            bool oneArtist;
            std::vector<Metadata::Song> songs;

            // Functions to create menus
            void createAlbumMenu();
            void createArtistsList();
            void createSongMenu(size_t);

            // Helper function to play album from position
            void playAlbum(size_t);

        public:
            // The constructor takes the ID of the artist to show
            Album(Main::Application *, AlbumID);

            // Delete created overlays
            ~Album();
    };
};

#endif