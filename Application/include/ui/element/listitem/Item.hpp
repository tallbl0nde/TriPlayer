#ifndef ELEMENT_LISTITEM_ITEM_HPP
#define ELEMENT_LISTITEM_ITEM_HPP

#include "Aether/Aether.hpp"

namespace CustomElm::ListItem {
    // Item extends a base Aether::AsyncItem to show a line either side of
    // the element.
    class Item : public Aether::AsyncItem {
        private:
            // Static line texture used to separate all items (instead of having multiple)
            static Aether::Drawable * line;

           // Note: positionElements() isn't defined by this class!

        protected:
            // Helper to update text
            void processText(Aether::Text * &, std::function<Aether::Text * ()>);

        public:
            // The constructor will create a line texture if one isn't present
            // Optionally takes height of item
            Item(int = 100);

            // Set colour to tint line with
            void setLineColour(const Aether::Colour &);

            // Render lines before children
            void render();
    };
};

#endif
