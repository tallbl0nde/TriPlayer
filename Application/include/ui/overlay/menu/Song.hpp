#ifndef OVERLAY_MENU_SONG_HPP
#define OVERLAY_MENU_SONG_HPP

#include "ui/element/MenuButton.hpp"
#include "ui/overlay/menu/Menu.hpp"

namespace CustomOvl::Menu {
    // Overlay shown when pressing the dots next to a song
    class Song : public Menu {
        public:
            // Type of song menu to create
            enum class Type {
                HideRemove,     // Don't show the 'Remove from Queue' button
                ShowRemove      // Show the 'Remove from Queue' button
            };

        private:
            // Elements
            Aether::Image * album;
            Aether::Text * title;
            Aether::Text * artist;
            Aether::Container * btns;
            CustomElm::MenuButton * removeFromQueue;
            CustomElm::MenuButton * addToQueue;
            CustomElm::MenuButton * addToPlaylist;
            CustomElm::MenuButton * goToArtist;
            CustomElm::MenuButton * goToAlbum;
            CustomElm::MenuButton * viewInformation;

        public:
            // Pass true to show 'Remove from Queue'
            Song(Type);

            // Move the highlight back to the top element
            void resetHighlight();

            // Set values
            void setAlbum(Aether::Image *);
            void setTitle(std::string);
            void setArtist(std::string);
            void setRemoveFromQueueText(std::string);
            void setAddToQueueText(std::string);
            void setAddToPlaylistText(std::string);
            void setGoToArtistText(std::string);
            void setGoToAlbumText(std::string);
            void setViewInformationText(std::string);

            // Set callbacks
            void setRemoveFromQueueFunc(std::function<void()>);
            void setAddToQueueFunc(std::function<void()>);
            void setAddToPlaylistFunc(std::function<void()>);
            void setGoToArtistFunc(std::function<void()>);
            void setGoToAlbumFunc(std::function<void()>);
            void setViewInformationFunc(std::function<void()>);

            // Set colours
            void setIconColour(Aether::Colour);
            void setMutedTextColour(Aether::Colour);
            void setTextColour(Aether::Colour);

            // Draw line texture three times
            void render();
    };
};

#endif