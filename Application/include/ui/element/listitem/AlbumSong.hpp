#ifndef ELEMENT_LISTITEM_ALBUMSONG_HPP
#define ELEMENT_LISTITEM_ALBUMSONG_HPP

#include "ui/element/listitem/More.hpp"

namespace CustomElm::ListItem {
    class AlbumSong : public More {
        private:
            // Texts
            Aether::Text * track;
            Aether::Text * title;
            Aether::Text * artist;
            Aether::Text * length;
            void positionElements();

        public:
            // Constructor sets up elements
            AlbumSong();

            // Set stuff
            void setTrackString(std::string);
            void setTitleString(std::string);
            void setArtistString(std::string);
            void setAlbumString(std::string);
            void setLengthString(std::string);
            void setTextColour(Aether::Colour);
    };
};

#endif
