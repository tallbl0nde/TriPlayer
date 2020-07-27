#ifndef FRAME_PLAYLISTS_HPP
#define FRAME_PLAYLISTS_HPP

#include "ui/frame/Frame.hpp"

// Forward declarations as only pointers are used in this header
namespace CustomOvl {
    class FileBrowser;
    class ItemMenu;
    class NewPlaylist;
};

namespace Frame {
    class Playlists : public Frame {
        private:
            // Various overlays shown
            CustomOvl::FileBrowser * browser;
            CustomOvl::ItemMenu * menu;
            Aether::MessageBox * msgbox;
            CustomOvl::NewPlaylist * newMenu;

            // Various things
            bool checkFB;
            Aether::Text * emptyMsg;
            Aether::FilledButton * newButton;

            // Vector of cached playlist metadata
            std::vector<Metadata::Playlist> metadata;
            // Metadata of playlist to create
            Metadata::Playlist newData;

            // Helper function to prepare menus
            void createDeletePlaylistMenu(const size_t);
            void createFileBrowser();
            void createMenu(size_t);
            void createNewPlaylistMenu();
            void createInfoOverlay(const std::string &);

            // Refreshes the list
            void refreshList();

            // Save new playlist to DB
            void savePlaylist();

        public:
            // Constructor sets strings and forms list using database
            Playlists(Main::Application *);

            // Override to update list whenever set active (not ideal but it works)
            void setActive();

            // Handles checking for file browser things
            void update(uint32_t);

            // Delete menu if there is one
            ~Playlists();
    };
};

#endif