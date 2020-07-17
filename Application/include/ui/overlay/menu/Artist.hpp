#ifndef OVERLAY_MENU_ARTIST_HPP
#define OVERLAY_MENU_ARTIST_HPP

#include "ui/element/MenuButton.hpp"
#include "ui/overlay/menu/Menu.hpp"

namespace CustomOvl::Menu {
    // Overlay shown when pressing the dots next to a song
    class Artist : public Menu {
        private:
            // Elements
            Aether::Container * btns;
            Aether::Image * image;
            Aether::Text * name;
            Aether::Text * stats;
            CustomElm::MenuButton * playAll;
            CustomElm::MenuButton * addToQueue;
            CustomElm::MenuButton * addToPlaylist;
            CustomElm::MenuButton * viewInformation;

        public:
            // Renders + positions elements
            Artist(Type);

            // Resets highlight to top button
            void resetHighlight();

            // Set values
            void setImage(Aether::Image *);
            void setName(std::string);
            void setStats(std::string);
            void setPlayAllText(std::string);
            void setAddToQueueText(std::string);
            void setAddToPlaylistText(std::string);
            void setViewInformationText(std::string);

            // Set callbacks
            void setPlayAllFunc(std::function<void()>);
            void setAddToQueueFunc(std::function<void()>);
            void setAddToPlaylistFunc(std::function<void()>);
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