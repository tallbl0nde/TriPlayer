#ifndef FRAME_SEARCH_HPP
#define FRAME_SEARCH_HPP

#include "ui/frame/Frame.hpp"
#include "ui/overlay/Overlay.hpp"

namespace Frame {
    class Search : public Frame {
        private:
            // Cached songIDs (used to set play queue)
            std::vector<SongID> songIDs;

            // Is the list empty?
            bool listEmpty;

            // Functions that setup/add relevant entries to the list
            void addEntries();
            void addPlaylists();
            void addArtists();
            void addAlbums();
            void addSongs();

            // Shows an error message in the middle of the frame
            void showError(const std::string &);

            // Function run by other thread to actually search the database
            bool searchDatabase(const std::string &);

            // === Variables used to operate the search thread ===
            // These vectors are filled with the results and emptied after use
            std::vector<Metadata::Playlist> playlists;
            std::vector<Metadata::Artist> artists;
            std::vector<Metadata::Album> albums;
            std::vector<Metadata::Song> songs;

            // Future returning true if search appeared to succeed
            std::future<bool> searchThread;

            // Set true after the thread is done to avoid accessing an invalid future
            bool threadDone;

        public:
            // Constructor sets up elements and invokes keyboard
            Search(Main::Application *);

            // Checks if the thread is finished
            void update(uint32_t);

            // Delete created menu
            ~Search();
    };
};

#endif