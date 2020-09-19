#ifndef FRAME_QUEUE_HPP
#define FRAME_QUEUE_HPP

#include <list>
#include "ui/frame/Frame.hpp"

// Forward declarations as these are used within the frame
namespace CustomElm::ListItem {
    class Song;
};
namespace CustomOvl {
    class ItemMenu;
}

namespace Frame {
    class Queue : public Frame {
        private:
            // Section in list
            enum class Section {
                Playing,
                Queue,
                UpNext
            };

            // Elements in list
            Aether::Element * playing;
            CustomElm::ListItem::Song * playingElm;
            Aether::Element * queue;
            std::list<CustomElm::ListItem::Song *> queueEls;
            Aether::Element * upnext;
            Aether::Text * upnextStr;
            std::list<CustomElm::ListItem::Song *> upnextEls;

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

            // Menu displayed when a song's "dots" are pressed
            CustomOvl::ItemMenu * menu;

            // Helper function to create song menu
            void createMenu(SongID, size_t, Section);

            // Prepare empty list
            void createList();
            // Hide list element and show empty message
            void initEmpty();
            // Update list elements
            void updateList();

            // Create a ListItem::Song for given id
            CustomElm::ListItem::Song * getListSong(size_t, Section);

        public:
            // Constructor sets strings and forms list using database and queue
            Queue(Main::Application *);

            // Update colours
            void updateColours();

            // Update the list when the queue changes
            void update(uint32_t);

            // Delete created menu
            ~Queue();
    };
};

#endif