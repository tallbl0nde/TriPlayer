#ifndef ELEMENT_LISTITEM_FILE_HPP
#define ELEMENT_LISTITEM_FILE_HPP

#include "ui/element/listitem/Item.hpp"

namespace CustomElm::ListItem {
    // This element is used in FileBrowser and represents an item
    // within a directory
    class File : public Item {
        private:
            // Elements
            Aether::Image * icon;
            Aether::Text * name;
            void positionElements();

        public:
            // Constructor takes name, if directory and callback
            File(std::string, bool, std::function<void()>);

            // Scroll text if highlighted
            void update(uint32_t);

            // Set colours
            void setIconColour(Aether::Colour);
            void setTextColour(Aether::Colour);
    };
};

#endif
