#ifndef ELEMENT_LISTITEM_ITEM_HPP
#define ELEMENT_LISTITEM_ITEM_HPP

#include "Aether/Aether.hpp"

namespace CustomElm::ListItem {
    // Item extends a base Aether::Element in order to provide automatic 'deferring'
    // functionality for items in a list. It does this by having a variable which
    // keeps track of when all it's textures are rendered.
    class Item : public Aether::Element {
        private:
            // Status of texture rendering
            enum class Status {
                Waiting,        // Not rendering and outside of threshold
                InProgress,     // Rendering all children textures
                Done            // Textures are visible and within threshold
            };
            Status renderStatus;

            // Static line texture used to separate all items (instead of having multiple)
            static SDL_Texture * line;
            Aether::Colour lineColour;

            // Vector of child textures to watch
            std::vector<Aether::Texture *> textures;

            // Children must implement this function which positions items
            // once they have all finished rendering
            virtual void positionItems() = 0;

        protected:
            // Mark a texture to be watched (also adds as child)
            void watchTexture(Aether::Texture *);

        public:
            // The constructor will create a line texture if one isn't present
            // Optionally takes height of item
            Item(int = 100);

            // Set colour to tint line with
            void setLineColour(Aether::Colour);

            // Check and update texture rendering things
            void update(uint32_t);

            // Render lines before children
            void render();

            // Call positionItems on width change
            void setW(int);
    };
};

#endif