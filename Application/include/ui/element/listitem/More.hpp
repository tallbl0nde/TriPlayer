#ifndef ELEMENT_LISTITEM_MORE_HPP
#define ELEMENT_LISTITEM_MORE_HPP

#include "ui/element/listitem/Item.hpp"

namespace CustomElm::ListItem {
    // 'More' extends the base item by handling the more button
    class More : public Item {
        private:
            // Related variables
            std::function<void()> moreCallback;
            bool touchedMore;

           // Note: positionElements() isn't defined by this class!

        protected:
            //'More' image (child should position it)
            Aether::Image * more;

        public:
            // Constructor accepts height
            More(int = 100);

            // Checks for X event or more touched
            bool handleEvent(Aether::InputEvent *);

            // Reset state when set inactive
            void setInactive();

            // Set callback when more is pressed
            void setMoreCallback(std::function<void()>);
            void setMoreColour(Aether::Colour);
    };
};

#endif
