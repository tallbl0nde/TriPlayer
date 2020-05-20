#ifndef FRAME_SONGS_HPP
#define FRAME_SONGS_HPP

#include "Frame.hpp"

namespace Frame {
    class Songs : public CustomElm::Frame {
        private:
            std::vector<SongID> songIDs;

        public:
            // Constructor sets strings and forms list using database
            Songs(Main::Application *);
    };
};

#endif