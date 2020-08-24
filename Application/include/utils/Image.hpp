#ifndef UTILS_IMAGE_HPP
#define UTILS_IMAGE_HPP

#include <vector>

namespace Utils::Image {
    // Resizes the given image to the provided dimensions
    // The buffer will be replaced with the resized image as a PNG
    // Accepts raw PNG/JPEG file, resized width, resized height
    // Returns true on success, false on an error
    bool resize(std::vector<unsigned char> &, size_t, size_t);
};

#endif