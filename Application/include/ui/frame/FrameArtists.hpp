#ifndef FRAME_ARTISTS_HPP
#define FRAME_ARTISTS_HPP

#include "ui/element/Frame.hpp"

namespace Frame {
    class Artists : public CustomElm::Frame {
        public:
            // Constructor sets strings and forms list using database
            Artists(Main::Application *);
    };
};

#endif