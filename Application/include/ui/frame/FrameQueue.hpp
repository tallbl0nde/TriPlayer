#ifndef FRAME_QUEUE_HPP
#define FRAME_QUEUE_HPP

#include "Frame.hpp"
#include "ListSong.hpp"

namespace Frame {
    class Queue : public CustomElm::Frame {
        private:
            // Top element in list
            std::vector<CustomElm::ListSong *> elements;
            // Vector for IDs in queue
            std::vector<SongID> songIDs;
            // Temporary vector of SongInfo
            std::vector<SongInfo> songInfo;

            // Empty message
            Aether::Text * emptyMsg;

            // "Cached" variables for updating on event
            size_t songIdx;
            bool songPressed;

            // Hide list element and show empty message
            void initEmpty();
            // Recreate list from scratch
            void initList();
            // Set 'remaining' strings (also makes sure songIdx is up to date)
            void updateStrings();

            // Create a ListSong for given index in vector
            CustomElm::ListSong * getListSong(size_t);

        public:
            // Constructor sets strings and forms list using database and queue
            Queue(Main::Application *);

            // Update the list when the queue changes
            void update(uint32_t);
    };
};

#endif