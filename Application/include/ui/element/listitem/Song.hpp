#ifndef ELEMENT_LISTITEM_SONG_HPP
#define ELEMENT_LISTITEM_SONG_HPP

#include "ui/element/listitem/More.hpp"

namespace CustomElm::ListItem {
    // Contains song title, artist, album and duration
    class Song : public More {
        private:
            // Text elements
            Aether::Text * title;
            Aether::Text * artist;
            Aether::Text * album;
            Aether::Text * length;
            void positionElements();

        public:
            // Constructor sets up elements
            Song();

            // Set stuff
            void setTitleString(std::string);
            void setArtistString(std::string);
            void setAlbumString(std::string);
            void setLengthString(std::string);
            void setTextColour(Aether::Colour);
    };
};

#endif
