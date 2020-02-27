#ifndef ELEMENT_LISTSONG_HPP
#define ELEMENT_LISTSONG_HPP

#include "Aether.hpp"

namespace CustomElm {
    class ListSong : public Aether::Element {
        private:
            // Texts
            Aether::Text * title;
            Aether::Text * artist;
            Aether::Text * album;
            Aether::Text * length;

            // Top/bottom lines
            Aether::Rectangle * top;
            Aether::Rectangle * bottom;

            // Position items on x axis
            void positionItems();

        public:
            // Constructor sets up elements
            ListSong();

            // Set strings
            void setTitleString(std::string);
            void setArtistString(std::string);
            void setAlbumString(std::string);
            void setLengthString(std::string);

            // Set colours
            void setLineColour(Aether::Colour);
            void setTextColour(Aether::Colour);

            // Reposition elements on width change
            void setW(int);
    };
};

#endif