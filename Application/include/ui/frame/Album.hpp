#ifndef FRAME_ALBUM_HPP
#define FRAME_ALBUM_HPP

#include "Types.hpp"
#include "ui/frame/Frame.hpp"

// Forward declaration cause we don't need the include here
namespace CustomOvl::Menu {
    class Album;
};

namespace Frame {
    class Album : public Frame {
        private:
            // Menu shown when 'more' is pressed
            CustomOvl::Menu::Album * menu;

            // Functions to create menus
            void createAlbumMenu(AlbumID);

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