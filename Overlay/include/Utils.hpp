#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

// A Bitmap is a vector of pixels, and the dimensions associated
// with it as the vector is 1D.
struct Bitmap {
    std::vector<uint8_t> pixels;        // Array of pixel data
    unsigned char channels;             // Number of bytes forming one pixel
    unsigned int width;                 // Width of image (in pixels)
    unsigned int height;                // Height of image (in pixels)
};

namespace Utils {
    // Read a PNG from the specified buffer and convert to bitmap
    // Passed required size and performs scaling as it's extracted
    // Returns a Bitmap with no pixels on an error
    Bitmap convertPNGToBitmap(const std::vector<uint8_t> &, const size_t, const size_t);

    // Format seconds in HH:MM:SS
    std::string secondsToHMS(unsigned int);
};

#endif