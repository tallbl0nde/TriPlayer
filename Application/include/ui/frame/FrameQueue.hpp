#ifndef FRAME_QUEUE_HPP
#define FRAME_QUEUE_HPP

#include "Frame.hpp"
#include <list>
#include "ListSong.hpp"

namespace Frame {
    class Queue : public CustomElm::Frame {
        // Section in list
        enum class Section {
            Playing,
            Queue,
            UpNext
        };

        private:
            // Elements in list
            Aether::Element * playing;
            CustomElm::ListSong * playingElm;
            Aether::Element * queue;
            std::list<CustomElm::ListSong *> queueEls;
            Aether::Element * upnext;
            std::list<CustomElm::ListSong *> upnextEls;

            // Temporary vector of song metadata
            std::vector<Metadata::Song> songMeta;

            // Empty message
            Aether::Text * emptyMsg;

            // Was an item pressed?
            bool songPressed;

            // "Cached" variables for updating
            size_t cachedSongIdx;
            SongID cachedSongID;
            std::vector<SongID> cachedQueue;
            std::vector<SongID> cachedSubQueue;

            // Prepare empty list
            void createList();
            // Hide list element and show empty message
            void initEmpty();
            // Update list elements
            void updateList();

            // Create a ListSong for given id
            CustomElm::ListSong * getListSong(size_t, Section);

        public:
            // Constructor sets strings and forms list using database and queue
            Queue(Main::Application *);

            // Update the list when the queue changes
            void update(uint32_t);
    };
};

#endif