#ifndef FRAME_ARTIST_HPP
#define FRAME_ARTIST_HPP

#include "Types.hpp"
#include "ui/frame/Frame.hpp"

namespace Frame {
    class Artist : public Frame {
        public:
            // The constructor takes the ID of the artist to show
            Artist(Main::Application *, ArtistID);
    };
};

#endif