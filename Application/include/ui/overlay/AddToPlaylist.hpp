#ifndef OVERLAY_ADDTOPLAYLIST_HPP
#define OVERLAY_ADDTOPLAYLIST_HPP

#include "ui/element/listitem/Playlist.hpp"
#include "ui/overlay/Overlay.hpp"
#include "Types.hpp"

namespace CustomOvl {
    // The AddToPlaylist overlay presents a list of playlists
    // and provides the given callback with the ID of the chosen
    // playlist (set negative if cancelled)
    class AddToPlaylist : public Overlay {
        private:
            // Elements
            Aether::Rectangle * bg;
            Aether::Text * heading;
            Aether::Rectangle * line;
            Aether::List * list;

            // Callback for when a playlist is chosen
            std::function<void(PlaylistID)> chosenCallback;

        public:
            // Constructor initializes overlay
            AddToPlaylist();

            // Set stuff
            void setBackgroundColour(Aether::Colour);
            void setHeadingColour(Aether::Colour);
            void setHeadingString(const std::string &);
            void setLineColour(Aether::Colour);

            // Add a playlist element to the list (associates given ID with it)
            void addPlaylist(CustomElm::ListItem::Playlist *, PlaylistID);

            // Set the callback function which is called when a playlist is chosen
            void setChosenCallback(std::function<void(PlaylistID)>);
    };
};

#endif