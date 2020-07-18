#ifndef FRAME_ARTISTINFO_HPP
#define FRAME_ARTISTINFO_HPP

#include "Types.hpp"
#include "ui/element/TextBox.hpp"
#include "ui/frame/Frame.hpp"

namespace Frame {
    class ArtistInfo : public Frame {
        public:
            // The constructor takes the ID of the artist to show
            ArtistInfo(Main::Application *, ArtistID);
    };
};

#endif