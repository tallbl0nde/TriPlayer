#ifndef FRAME_ARTIST_HPP
#define FRAME_ARTIST_HPP

#include "Types.hpp"
#include "ui/frame/Frame.hpp"
#include "ui/overlay/menu/Artist.hpp"

namespace Frame {
    class Artist : public Frame {
        private:
            // Menu shown when 'more' is pressed
            CustomOvl::Menu::Artist * menu;

        public:
            // The constructor takes the ID of the artist to show
            Artist(Main::Application *, ArtistID);

            // Delete created overlays
            ~Artist();
    };
};

#endif