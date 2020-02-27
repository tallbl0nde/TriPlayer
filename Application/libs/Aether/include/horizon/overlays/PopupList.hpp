#ifndef AETHER_POPUPLIST_HPP
#define AETHER_POPUPLIST_HPP

#include "Overlay.hpp"
#include "horizon/controls/Controls.hpp"
#include "horizon/list/List.hpp"
#include "horizon/list/ListButton.hpp"
#include "primary/Rectangle.hpp"

namespace Aether {
    // A "PopupList" is an overlay at the bottom of the screen containing
    // a list of items to select from. A callback must be specified for each
    // item, and the overlay is closed when any item is selected (not reauired
    // to be specified within callbacks!)
    class PopupList : public Overlay {
        private:
            // Store pointers in order to reposition
            Controls * ctrl;
            List * list;
            Rectangle * rect, * top, * bottom;
            Text * title;

            // Need to keep pointers to list items to change colours
            std::vector<ListButton *> items;

            // Need to store colours for list
            Colour hiColour, llColour, txColour;

        public:
            // Constructor takes title string
            PopupList(std::string);

            // Add the given item string + callback to the list (optional parameter set to true if ticked)
            void addEntry(std::string, std::function<void()>, bool = false);

            // Removes all entries
            void removeEntries();

            // Getters + setters colours
            Colour getBackgroundColour();
            void setBackgroundColour(Colour);
            Colour getTextColour();
            void setTextColour(Colour);
            Colour getLineColour();
            void setLineColour(Colour);
            Colour getHighlightColour();
            void setHighlightColour(Colour);
            Colour getListLineColour();
            void setListLineColour(Colour);

            // Set all colours
            // BG, Highlighted, Line, List Line, Text
            void setAllColours(Colour, Colour, Colour, Colour, Colour);
    };
};

#endif