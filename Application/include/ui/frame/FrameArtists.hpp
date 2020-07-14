#ifndef FRAME_ARTISTS_HPP
#define FRAME_ARTISTS_HPP

#include "ui/element/Frame.hpp"
#include "ui/overlay/menu/Artist.hpp"

namespace Frame {
    class Artists : public CustomElm::Frame {
        private:
            // Menu displayed when the dots are pressed
            CustomOvl::Menu::Artist * menu;

            // Helper function to prepare menu
            void createMenu(ArtistID);

        public:
            // Constructor sets strings and forms list using database
            Artists(Main::Application *);

            // Delete menu if there is one
            ~Artists();
    };
};

#endif