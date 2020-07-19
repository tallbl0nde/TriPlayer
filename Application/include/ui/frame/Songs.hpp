#ifndef FRAME_SONGS_HPP
#define FRAME_SONGS_HPP

#include "ui/frame/Frame.hpp"
#include "ui/overlay/menu/Song.hpp"

namespace Frame {
    class Songs : public Frame {
        private:
            // Cached songIDs (used to set play queue)
            std::vector<SongID> songIDs;

            // Menu displayed when a song's "dots" are pressed
            CustomOvl::Menu::Song * menu;

            // Helper function to create song menu
            void createMenu(SongID);

        public:
            // Constructor sets strings and forms list using database
            Songs(Main::Application *);

            // Delete created menu
            ~Songs();
    };
};

#endif