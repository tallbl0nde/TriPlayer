#include <cstring>
#include <png.h>
#include "Utils.hpp"

// Wraps a vector to provide 'stream'-like functionality
struct VectorStream {
    const std::vector<unsigned char> & buffer;  // Vector
    size_t pos;                                 // Position of next byte to read
};

namespace Utils {
    // Callback for libpng to read from buffer
    void readDataFromBuffer(png_structp png, png_bytep bytes, png_size_t amount) {
        // Get pointer to vector
        png_voidp io_ptr = png_get_io_ptr(png);
        if (io_ptr == nullptr) {
            return;
        }
        VectorStream * data = (VectorStream *)io_ptr;

        // Read bytes
        size_t remaining = data->buffer.size() - data->pos;
        if (amount > remaining) {
            amount = remaining;
        }
        std::memcpy(bytes, &data->buffer[data->pos], amount);
        data->pos += amount;
    }

    Bitmap convertPNGToBitmap(const std::vector<uint8_t> & buffer) {
        Bitmap bitmap;
        bitmap.channels = 0;
        bitmap.width = 0;
        bitmap.height = 0;

        // Check if actually a png
        if (png_sig_cmp(&buffer[0], 0, 8)) {
            return bitmap;
        }

        // If OK, proceed by creating a "stream" for the buffer
        VectorStream input = {buffer, 0};

        // Initialize library
        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (png == nullptr) {
            return bitmap;
        }
        png_infop info = png_create_info_struct(png);
        if (info == nullptr) {
            png_destroy_read_struct(&png, nullptr, nullptr);
            return bitmap;
        }
        png_set_read_fn(png, (void *) &input, readDataFromBuffer);

        // Read PNG header to extract important info
        png_read_info(png, info);
        int bitDepth;
        int colourType;
        png_uint_32 rc = png_get_IHDR(png, info, &bitmap.width, &bitmap.height, &bitDepth, &colourType, nullptr, nullptr, nullptr);
        if (rc != 1) {
            png_destroy_read_struct(&png, &info, nullptr);
            return bitmap;
        }
        switch (colourType) {
            case PNG_COLOR_TYPE_RGB:
                png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
                png_read_update_info(png, info);

            case PNG_COLOR_TYPE_RGB_ALPHA:
                bitmap.channels = 4;
                break;

            default:
                return bitmap;
        }

        // Now actually extract the pixels
        png_uint_32 bytesPerRow = png_get_rowbytes(png, info);
        uint8_t * rowData = new uint8_t[bytesPerRow];

        // Iterate over each row of pixels
        for (png_uint_32 row = 0; row < bitmap.height; row++) {
            // Read the pixels
            png_read_row(png, (png_bytep)rowData, nullptr);

            // Append row to end of buffer
            bitmap.pixels.resize(bitmap.pixels.size() + bytesPerRow, 0);
            std::memcpy(&bitmap.pixels[row * bytesPerRow], rowData, bytesPerRow);
        }

        // Tidy up and return
        delete[] rowData;
        bitmap.pixels.shrink_to_fit();
        png_destroy_read_struct(&png, &info, nullptr);
        return bitmap;
    }

    bool resizeBitmap(Bitmap & bitmap, const size_t newWidth, const size_t newHeight) {
        // Sanity check
        if (bitmap.pixels.empty()) {
            return false;
        }

        // Do nothing if same size
        if (bitmap.width == newWidth && bitmap.height == newHeight) {
            return true;
        }

        // Create the vector of resized pixels
        std::vector<uint8_t> resized;

        // Calculate ratios
        double xRatio = static_cast<double>(bitmap.width-1)/newWidth;
        double yRatio = static_cast<double>(bitmap.height-1)/newHeight;

        // Iterate over the original image for each pixel in the resized image
        for (size_t h = 0; h < newHeight; h++) {
            for (size_t w = 0; w < newWidth; w++) {
                // Find pixel(s) to examine
                int x = xRatio * w;
                int y = yRatio * h;
                double xDiff = (xRatio * w) - x;
                double yDiff = (yRatio * h) - y;
                size_t index = (y*bitmap.width + x)*bitmap.channels;

                // Iterate for each channel that forms a pixel
                for (size_t p = 0; p < bitmap.channels; p++) {
                    // Get pixels
                    uint8_t a = bitmap.pixels[index + p];
                    uint8_t b = bitmap.pixels[index + p + bitmap.channels];
                    uint8_t c = bitmap.pixels[index + p + bitmap.width * bitmap.channels];
                    uint8_t d = bitmap.pixels[index + p + (bitmap.width + 1) * bitmap.channels];

                    // Interpolate
                    uint8_t interpolated = a*(1-xDiff)*(1-yDiff) + b*(xDiff)*(1-yDiff) + c*(yDiff)*(1-xDiff) + d*(xDiff)*(yDiff);
                    resized.push_back(interpolated);
                }
            }
        }

        // Replace vector and return
        bitmap.pixels = resized;
        bitmap.width = newWidth;
        bitmap.height = newHeight;
        return true;
    }

    std::string secondsToHMS(unsigned int sec) {
        std::string str = "";
        // Hours
        int h = sec/3600;
        if (h > 0) {
            str += std::to_string(h) + ":";
        }

        // Minutes
        int m = ((sec/60)%60);
        if (str.length() > 0 && m < 10) {
            str += "0";
        }
        str += std::to_string(m);

        // Seconds
        str += ":";
        int s = sec%60;
        if (s < 10) {
            str += "0";
        }
        str += std::to_string(s);

        return str;
    }
};