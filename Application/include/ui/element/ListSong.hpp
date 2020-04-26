#ifndef ELEMENT_LISTSONG_HPP
#define ELEMENT_LISTSONG_HPP

#include "Aether.hpp"

namespace CustomElm {
    class ListSong : public Aether::Element {
        private:
            // Static texture for top/bottom lines
            // This works as all ListSong's will only be in the same list
            static SDL_Texture * lineTexture;
            Aether::Colour lineColour;

            // Texts
            Aether::Exp::ThreadedText * title;
            Aether::Exp::ThreadedText * artist;
            Aether::Exp::ThreadedText * album;
            Aether::Exp::ThreadedText * length;

            // Position items on x axis
            void positionItems();

        public:
            // Constructor sets up elements (takes pointer to ThreadQueue which is passed to strings)
            ListSong(Aether::ThreadQueue *);

            // Position text once rendered
            void update(uint32_t);

            // Render lineTexture as well as normal rendering
            void render();

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