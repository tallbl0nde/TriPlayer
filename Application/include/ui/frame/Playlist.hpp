#ifndef FRAME_PLAYLIST_HPP
#define FRAME_PLAYLIST_HPP

#include "Types.hpp"
#include "ui/frame/Frame.hpp"

// Forward declarations cause only pointers are needed here
namespace CustomOvl {
    class ItemMenu;
    class Menu;
};

namespace Frame {
    class Playlist : public Frame {
        private:
            // Menu shown when 'more' is pressed
            Aether::MessageBox * msgbox;
            CustomOvl::Menu * playlistMenu;
            CustomOvl::ItemMenu * songMenu;

            // Indicates frame should delete itself
            bool goBack;

            // Cached data
            Metadata::Playlist metadata;
            std::vector<Metadata::Song> songs;

            // Functions to create menus
            void createDeleteMenu();
            void createPlaylistMenu();
            void createSongMenu(size_t);

            // Helper function to play playlist from position
            void playPlaylist(size_t);

        public:
            // The constructor takes the ID of the playlist to show
            Playlist(Main::Application *, PlaylistID);

            // Check when we need to change frame
            void update(uint32_t);

            // Delete created overlays
            ~Playlist();
    };
};

#endif