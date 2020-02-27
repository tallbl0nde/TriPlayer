#ifndef AETHER_CONTAINER_HPP
#define AETHER_CONTAINER_HPP

#include "Element.hpp"

namespace Aether {
    // A container is an element that contains other elements. It is required
    // to automatically move between elements without having to specify which
    // ones are next to which.
    class Container : public Element {
        public:
            // Constructor is same as Element
            Container(int = 0, int = 0, int = 100, int = 100);

            void addElement(Element *);

            // handleEvent() will pass to children first and if nothing is handled
            // it will "deactivate" on element and "activate" the one it moves to
            bool handleEvent(InputEvent *);

            // Re-highlight/focus the focussed element
            void setActive();
            void setInactive();

            // Friend function to reduce similar code used for determining which element to move to
            // (parent element (usually this), function which returns true if valid, function to return distance between)
            friend bool moveHighlight(Container *, std::function<bool(Element *, Element *)>, std::function<int(Element *, Element *)>);
    };
};

#endif