#ifndef FRAME_SONGS_HPP
#define FRAME_SONGS_HPP

#include "db/Database.hpp"
#include "ui/frame/Frame.hpp"

// Forward declaration as the class is used within the frame
namespace CustomOvl {
    class ItemMenu;
    class SortBy;
}

namespace Frame {
    class Songs : public Frame {
        private:
            // Cached songIDs (used to set play queue)
            std::vector<SongID> songIDs;

            // Sort by menu
            CustomOvl::SortBy * sortMenu;

            // Menu displayed when a song's "dots" are pressed
            CustomOvl::ItemMenu * menu;

            // (Re)create list with given sorting order
            void createList(Database::SortBy);

            // Create the above menu
            void createMenu(SongID);

        public:
            // Constructor sets strings and forms list using database
            Songs(Main::Application *);

            // Delete created menu
            ~Songs();
    };
};

#endif