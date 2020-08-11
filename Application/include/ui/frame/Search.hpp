#ifndef FRAME_SEARCH_HPP
#define FRAME_SEARCH_HPP

#include "ui/frame/Frame.hpp"

namespace Frame {
    class Search : public Frame {
        private:
            // Cached songIDs (used to set play queue)
            std::vector<SongID> songIDs;

            // Is the list empty?
            bool listEmpty;

            // Functions that setup/add relevant entries to the list
            void addPlaylists(const std::vector<Metadata::Playlist> &);
            void addArtists(const std::vector<Metadata::Artist> &);
            void addAlbums(const std::vector<Metadata::Album> &);
            void addSongs(const std::vector<Metadata::Song> &);

            // Callback when text box is updated
            void searchDatabase(const std::string &);

            // Shows an error message
            void showError(const std::string &);

        public:
            // Constructor sets up elements and invokes keyboard
            Search(Main::Application *);

            // Delete created menu
            ~Search();
    };
};

#endif