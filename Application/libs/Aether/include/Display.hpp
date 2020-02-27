#ifndef AETHER_DISPLAY_HPP
#define AETHER_DISPLAY_HPP

#include "Overlay.hpp"
#include <stack>

// Enum for stack operation performed
enum class StackOp {
    Push,
    Pop,
    None
};

namespace Aether {
    // The Display represents the "root" element. It stores and handles different screens
    // and polling/passing events
    class Display : public Element {
        private:
            // Set to false to indicate termination of app
            bool loop_;
            // Set to true to show fps
            bool fps_;

            // Colour to clear screen with
            Colour bg;

            // Colours to highlight elements with
            std::function<Colour(uint32_t)> hiAnim;

            // Variables to handle held buttons
            Button heldButton;
            int heldTime;
            // Time between repeated button events (in ms)
            int holdDelay_;

            // Vector of overlays, drawn from start -> end but only last one gets events!
            std::vector<Overlay *> overlays;

            // Pointer to current screen to draw
            Screen * screen;
            // Pointer to screen to change to after loop
            Screen * nextScreen;
            // Stack of screens
            std::stack<Screen *> screenStack;
            // Set true on a stack operation to avoid on(Un)load calls
            StackOp stackOp;

            // Current fade alpha
            short fadeAlpha;

            // These functions are private members of a display
            // as they are called by loop()
            using Element::addElement;
            using Element::handleEvent;
            using Element::update;
            using Element::render;

        public:
            // Constructor takes no parameters
            // Initializes SDL with Vsync on
            Display();

            // Toggle showing fps
            void setShowFPS(bool);

            // Set colour to clear screen with
            void setBackgroundColour(uint8_t, uint8_t, uint8_t);
            // Set colours to highlight elements with
            void setHighlightColours(Colour, Colour);
            void setHighlightAnimation(std::function<Colour(uint32_t)>);

            // Set font for text
            void setFont(std::string);

            // Getter + setter for hold delay
            int holdDelay();
            void setHoldDelay(int);

            // Add an overlay
            void addOverlay(Overlay *);

            // Set screen to given pointer
            void setScreen(Screen *);
            // Push the current screen on a stack
            // onUnload (for current screen) is not called!!
            // Note that Aether will quit if another screen is not set before next call of loop()
            void pushScreen();
            // Pops a screen from the stack
            // onLoad (for stack screen) is not called!!
            // Note that Aether will not delete the current screen - it is up to you!
            void popScreen();

            // Call to fade in
            void setFadeIn();
            // Executes the main loop (events + rendering)
            // Returns false when the app should be terminated
            bool loop();

            // Sets loop to false and thus exit
            void exit();

            ~Display();
    };
};

#endif