#ifndef ELEMENT_BUTTON_HPP
#define ELEMENT_BUTTON_HPP

#include "tesla.hpp"
#include "Utils.hpp"

// A Button is a tesla element that contains an image. This image is passed
// to the button upon construction as a vector of bytes.
namespace Element {
    class Button : public tsl::elm::Element {
        private:
            Bitmap image;                       // Actual image to render
            Bitmap altImage;                    // Secondary image to swap to
            bool showAlt;                       // Whether to show alternate image
            tsl::Color colour;                  // Colour to draw image in

            unsigned short fontSize;            // Font size for text
            std::string text;                   // Text to draw
            bool set;                           // Whether dimensions have been set yet

            std::function<void()> callback;     // Function to call when pressed

        public:
            // Constructor accepts image buffer (as PNG) and padding
            Button(short, short, const std::vector<uint8_t> &);
            // Alternative constructor accepts padding, string + font size
            Button(short, short, const std::string &, unsigned int);

            // Add an optional second image to show
            void addAltImage(const std::vector<uint8_t> &);
            // Toggle between images
            void showAltImage(const bool);
            // Set colour to render image with
            void setColour(tsl::Color);

            // Call assigned callback
            void runCallback();
            // Set function to execute when tapped/pressed
            void setCallback(std::function<void()>);

            // Always return that we can be focussed
            tsl::elm::Element * requestFocus(tsl::elm::Element *, tsl::FocusDirection);
            // Override draw method to render image
            void draw(tsl::gfx::Renderer *);
            // Override layout method to do nothing :P
            void layout(u16, u16, u16, u16);
            // Wait for A to be pressed and call callback
            bool onClick(u64);
    };
};

#endif