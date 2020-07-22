#ifndef FRAME_ARTIST_HPP
#define FRAME_ARTIST_HPP

#include "Types.hpp"
#include "ui/frame/Frame.hpp"

// Forward declaration cause we don't need the include here
namespace CustomOvl::Menu {
    class Artist;
};

namespace Frame {
    class Artist : public Frame {
        private:
            // Menu shown when 'more' is pressed
            CustomOvl::Menu::Artist * menu;
            CustomOvl::Menu::Artist * albumMenu;    // We use an Artist menu here as it accomplishes the same thing

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