#ifndef ELEMENT_LISTITEM_PLAYLIST_HPP
#define ELEMENT_LISTITEM_PLAYLIST_HPP

#include "ui/element/listitem/More.hpp"

namespace CustomElm::ListItem {
    class Playlist : public More {
        private:
            // Elements
            bool showMore;
            Aether::Image * image;
            Aether::Text * name;
            Aether::Text * songs;
            void positionElements();

        public:
            // Constructor sets up elements (takes path to image)
            Playlist(const std::string &, const bool);

            // Scroll if needed
            void update(uint32_t);

            // Set stuff
            void setNameString(const std::string &);
            void setSongsString(const std::string &);
            void setMutedTextColour(Aether::Colour);
            void setTextColour(Aether::Colour);
    };
};

#endif
