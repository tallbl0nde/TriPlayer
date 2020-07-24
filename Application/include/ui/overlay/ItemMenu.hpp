#ifndef OVERLAY_ITEMMENU_HPP
#define OVERLAY_ITEMMENU_HPP

#include "ui/overlay/Menu.hpp"

namespace CustomOvl {
    // An item menu extends Menu to show an image at the top
    // along with some text
    class ItemMenu : public Menu {
        private:
            // Elements
            Aether::Element * top;
            Aether::Image * image;
            Aether::Text * mainText;
            Aether::Text * subText;

        public:
            // Constructor initializes everything to nullptr
            ItemMenu();

            // Set each of the elements (positioned automatically)
            void setImage(Aether::Image *);
            void setMainText(const std::string &);
            void setSubText(const std::string &);

            // Set colours
            void setMainTextColour(Aether::Colour);
            void setSubTextColour(Aether::Colour);
    };
};

#endif