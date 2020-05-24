#ifndef OVERLAY_SONGMENU_HPP
#define OVERLAY_SONGMENU_HPP

#include "MenuButton.hpp"

namespace CustomOvl {
    // Overlay shown when pressing the dots next to a song
    class SongMenu : public Aether::Overlay {
        private:
            // Elements
            Aether::Rectangle * bg;
            Aether::Image * album;
            Aether::Text * title;
            Aether::Text * artist;
            CustomElm::MenuButton * removeFromQueue;
            CustomElm::MenuButton * addToQueue;
            CustomElm::MenuButton * addToPlaylist;
            CustomElm::MenuButton * goToArtist;
            CustomElm::MenuButton * goToAlbum;
            CustomElm::MenuButton * viewDetails;

            // Separating line texture
            SDL_Texture * line;
            Aether::Colour lineCol;

        public:
            // Pass true to show 'Remove from Queue'
            SongMenu(bool);

            // Set values
            void setAlbum(Aether::Image *);
            void setTitle(std::string);
            void setArtist(std::string);
            void setRemoveFromQueueText(std::string);
            void setAddToQueueText(std::string);
            void setAddToPlaylistText(std::string);
            void setGoToArtistText(std::string);
            void setGoToAlbumText(std::string);
            void setViewDetailsText(std::string);

            // Set callbacks
            void setRemoveFromQueueFunc(std::function<void()>);
            void setAddToQueueFunc(std::function<void()>);
            void setAddToPlaylistFunc(std::function<void()>);
            void setGoToArtistFunc(std::function<void()>);
            void setGoToAlbumFunc(std::function<void()>);
            void setViewDetailsFunc(std::function<void()>);

            // Set colours
            void setBackgroundColour(Aether::Colour);
            void setIconColour(Aether::Colour);
            void setLineColour(Aether::Colour);
            void setMutedTextColour(Aether::Colour);
            void setTextColour(Aether::Colour);

            // Close if tapped outside of bg
            bool handleEvent(Aether::InputEvent *);

            // Draw line texture three times
            void render();

            // Delete line texture
            ~SongMenu();
    };
};

#endif