#ifndef ELEMENT_BUTTON_HPP
#define ELEMENT_BUTTON_HPP

#include "tesla.hpp"
#include "Utils.hpp"

// A Button is a tesla element that contains an image. This image is passed
// to the button upon construction as a vector of bytes.
namespace Element {
    class Button : public tsl::elm::Element {
        private:
            Bitmap image;               // Actual image to render
            Bitmap altImage;            // Secondary image to swap to
            bool showAlt;               // Whether to show alternate image
            tsl::Color colour;          // Colour to draw image in

        public:
            // Constructor accepts image buffer (as PNG) and padding
            Button(int, int, const std::vector<uint8_t> &);

            // Add an optional second image to show
            void addAltImage(const std::vector<uint8_t> &);

            // Toggle between images
            void showAltImage(const bool);

            // Always return that we can be focussed
            tsl::elm::Element * requestFocus(tsl::elm::Element *, tsl::FocusDirection);

            // Set colour to render image with
            void setColour(tsl::Color);

            // Override draw method to render image
            void draw(tsl::gfx::Renderer *);

            // Override layout method to do nothing :P
            void layout(u16, u16, u16, u16);
    };
};

#endif