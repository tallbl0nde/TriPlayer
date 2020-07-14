#ifndef FRAME_SONGS_HPP
#define FRAME_SONGS_HPP

#include "ui/frame/Frame.hpp"

namespace Frame {
    class Songs : public Frame {
        private:
            std::vector<SongID> songIDs;

        public:
            // Constructor sets strings and forms list using database
            Songs(Main::Application *);
    };
};

#endif