#ifndef AETHER_ELEMENT_HPP
#define AETHER_ELEMENT_HPP

#include "InputEvent.hpp"
#include <vector>

namespace Aether {
    // Element is the base class inherited from to form all other
    // types of elements. Thus, it contains positioning, rendering, callback, etc variables
    class Element {
        private:
            // Positioning and size variables
            int x_;
            int y_;
            int w_;
            int h_;

            // Skip rendering this element + it's children
            bool hidden_;

            // Function to call when element is tapped/selected
            std::function<void()> callback_;
            // Does this element contain a highlighted element?
            bool hasHighlighted_;
            // Does this element contain a selectable element?
            bool hasSelectable_;
            // Does this element contain the selected element?
            bool hasSelected_;
            // Is this element highlighted
            bool highlighted_;
            // Can this element be selected?
            bool selectable_;
            // Is this element selected? (ie. button held/touch held)
            bool selected_;
            // Can this element be touched?
            bool touchable_;

        protected:
            // Colours used for highlighting
            static Colour hiBG, hiBorder, hiSel;
            // Size of highlight border
            static unsigned int hiSize;
            // Set true if touch is "active" (i.e. hide highlighting)
            static bool isTouch;

            // Parent element
            // Only nullptr if root element (ie. display)
            Element * parent;

            // Vector of child elements (used to call their methods)
            std::vector<Element *> children;

            // Element which is highlighted/focussed (to regain focus on activation)
            Element * focussed_;

        public:
            // Constructor must be passed parent element (or nullptr if there is none)
            // Coordinates and size are optional, defaults to (0,0) with size (100, 100)
            Element(int = 0, int = 0, int = 100, int = 100);

            // Getters and setters for x, y, w, h + scale
            int x();
            int y();
            int w();
            int h();
            virtual void setX(int);
            virtual void setY(int);
            virtual void setW(int);
            virtual void setH(int);
            // Combines functions into one
            virtual void setXY(int, int);
            virtual void setWH(int, int);
            virtual void setXYWH(int, int, int, int);

            // Set given element as parent
            void setParent(Element *);
            // Add an element as a child
            virtual void addElement(Element *);
            // Delete the given child, returns false if not a child
            virtual bool removeElement(Element *);
            // Delete all children elements
            virtual void removeAllElements();

            // Returns whether this element is onscreen
            bool isVisible();

            // Returns hidden
            bool hidden();
            // Hide/show this element
            void setHidden(bool);

            // Returns true if selected
            bool selected();
            void setSelected(bool);
            // Returns selectable
            bool selectable();
            // Set whether element is selectable
            void setSelectable(bool);

            // Returns touchable
            bool touchable();
            // Set whether element is touchable
            void setTouchable(bool);

            bool highlighted();
            void setHighlighted(bool);

            bool hasHighlighted();
            void setHasHighlighted(bool);

            bool hasSelectable();
            void setHasSelectable(bool);

            bool hasSelected();
            void setHasSelected(bool);

            virtual void setActive();
            virtual void setInactive();
            void setFocussed(Element *);
            Element * focussed();

            // Returns callback function (nullptr if no callback assigned)
            std::function<void()> callback();
            // Set callback function (also marks element as selectable)
            void setCallback(std::function<void()>);

            // Handle the given event
            virtual bool handleEvent(InputEvent *);
            // Update is passed time since last frame (for animations)
            virtual void update(uint32_t);
            // Render child elements
            virtual void render();

            // Default are rectangles
            // Renders the highlight border + background
            virtual void renderHighlighted();
            // Renders the selected overlay
            virtual void renderSelected();

            // Destructor calls destructor of children
            virtual ~Element();

            // Returns the element currently highlighted within given element
            // or nullptr if none found
            friend void moveHighlight(Element *);
    };
};

#endif