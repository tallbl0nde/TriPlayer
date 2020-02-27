#ifndef AETHER_SCREEN_HPP
#define AETHER_SCREEN_HPP

#include "base/Container.hpp"

namespace Aether {
    // A screen represents a literal screen/layout. It stores all elements
    // to be drawn/interacted with on a single screen.
    class Screen : public Container {
        private:
            // Map of button > func for custom callbacks on button presses
            std::unordered_map<Button, std::function<void()> > pressFuncs;

            // Map of button > func for custom callbacks on button presses
            std::unordered_map<Button, std::function<void()> > releaseFuncs;

        public:
            // Constructor takes parent element and active/inactive functions
            Screen();

            // Functions that are called when screen is set/unser
            virtual void onLoad();
            virtual void onUnload();

            // Set a the given callback to the given button press
            // Note that setting a button callback will block the event from
            // going to any other elements!!
            void onButtonPress(Button, std::function<void()>);
            void onButtonRelease(Button, std::function<void()>);

            // Check if callback is set and execute, otherwise pass event
            // to elements
            bool handleEvent(InputEvent *);
    };
};

#endif