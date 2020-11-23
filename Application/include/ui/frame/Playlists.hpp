#ifndef FRAME_PLAYLISTS_HPP
#define FRAME_PLAYLISTS_HPP

#include "db/Database.hpp"
#include "ui/frame/Frame.hpp"

// Forward declarations as only pointers are used in this header
namespace CustomElm::ListItem {
    class Playlist;
};

namespace CustomOvl {
    class FileBrowser;
    class ItemMenu;
    class NewPlaylist;
    class SortBy;
};

namespace Frame {
    class Playlists : public Frame {
        private:
            // Struct forming a pair between metadata and list item
            struct Item {
                Metadata::Playlist meta;                // Playlist metadata
                CustomElm::ListItem::Playlist * elm;    // List item matching metadata
            };

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
            std::vector<Item> items;
            size_t pushedIdx;

            // Metadata of playlist to create
            Metadata::Playlist newData;

            // Sort menu and last type
            CustomOvl::SortBy * sortMenu;
            Database::SortBy sortType;

            // Helper function to prepare menus
            void createDeletePlaylistMenu(const size_t);
            void createFileBrowser();
            void createMenu(size_t);
            void createNewPlaylistMenu();
            void createInfoOverlay(const std::string &);

            // Creates a ListItem::Playlist from the given metadata
            CustomElm::ListItem::Playlist * getListItem(const Metadata::Playlist &);

            // Export the chosen playlist as a .m3u8
            void exportPlaylist(const Metadata::Playlist &);

            // Reconstructs the entire list from scratch
            void refreshList(Database::SortBy);

            // Save new playlist to DB
            void savePlaylist();

        public:
            // Constructor sets strings and forms list using database
            Playlists(Main::Application *);

            // Update colours
            void updateColours();

            // Handles checking for file browser things
            void update(uint32_t);

            // Check for changes when popped
            void onPop(Type);

            // Delete menu if there is one
            ~Playlists();
    };
};

#endif