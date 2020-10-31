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
    // Returns a Bitmap with no pixels on an error
    Bitmap convertPNGToBitmap(const std::vector<uint8_t> &);

    // Resizes a bitmap in place using bilinear interpolation
    // Returns whether successful or not
    bool resizeBitmap(Bitmap &, const size_t, const size_t);
};

#endif