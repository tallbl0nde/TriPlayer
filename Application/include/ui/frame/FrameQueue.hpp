#ifndef QUEUE_HPP
#define QUEUE_HPP

#include "Frame.hpp"

namespace Frame {
    class Queue : public CustomElm::Frame {
        private:
            std::vector<SongID> songIDs;

        public:
            // Constructor sets strings and forms list using database and queue
            Queue(Main::Application *);
    };
};

#endif