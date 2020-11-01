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

    Bitmap convertPNGToBitmap(const std::vector<uint8_t> & buffer, const size_t scaleWidth, const size_t scaleHeight) {
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
            // Append alpha channel if the image doesn't have one
            case PNG_COLOR_TYPE_RGB:
                png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
                png_read_update_info(png, info);

            case PNG_COLOR_TYPE_RGB_ALPHA:
                bitmap.channels = 4;
                break;

            default:
                return bitmap;
        }

        // Now actually extract the pixels (+ scale)
        png_uint_32 bytesPerRow = png_get_rowbytes(png, info);
        uint8_t * rowData = new uint8_t[bytesPerRow];

        // Simply copy if not scaling
        if (scaleWidth == 0 || scaleHeight == 0) {
            for (size_t row = 0; row < bitmap.height; row++) {
                png_read_row(png, (png_bytep)rowData, nullptr);
                bitmap.pixels.resize(bitmap.pixels.size() + bytesPerRow);
                std::memcpy(&bitmap.pixels[row * bytesPerRow], rowData, bytesPerRow);
            }

        // Otherwise scale while we extract!
        } else {
            // Calculate ratios
            double xRatio = static_cast<double>(bitmap.width-1)/scaleWidth;
            double yRatio = static_cast<double>(bitmap.height-1)/scaleHeight;

            // Iterate over each required row in the scaled image
            // If we need data from a row that hasn't been read in yet, read it in
            // Once we are past rows that we don't need anymore, delete them from memory
            std::vector< std::vector<uint8_t> > rows;
            for (size_t h = 0; h < scaleHeight; h++) {
                size_t y = yRatio * h;
                double yDiff = (yRatio * h) - y;

                // Read in any rows we need to read from
                while (rows.size() <= y+1) {
                    std::vector<uint8_t> row;
                    row.resize(bytesPerRow);
                    png_read_row(png, (png_bytep)rowData, nullptr);
                    std::memcpy(&row[0], rowData, bytesPerRow);
                    rows.push_back(row);
                }

                // Interpolate the current row's pixels
                for (size_t w = 0; w < scaleWidth; w++) {
                    size_t x = xRatio * w;
                    double xDiff = (xRatio * w) - x;
                    size_t index = x*bitmap.channels;

                    // Iterate for each channel that forms a pixel
                    for (size_t p = 0; p < bitmap.channels; p++) {
                        // Get pixels
                        uint8_t a = rows[y][index + p];
                        uint8_t b = rows[y][index + p + bitmap.channels];
                        uint8_t c = rows[y+1][index + p];
                        uint8_t d = rows[y+1][index + p + bitmap.channels];

                        // Interpolate
                        uint8_t interpolated = a*(1-xDiff)*(1-yDiff) + b*(xDiff)*(1-yDiff) + c*(yDiff)*(1-xDiff) + d*(xDiff)*(yDiff);
                        bitmap.pixels.push_back(interpolated);
                    }
                }

                // Discard any rows that are no longer needed
                for (size_t r = 0; r < y; r++) {
                    if (!rows[r].empty()) {
                        std::vector<uint8_t>().swap(rows[r]);
                    }
                }
            }
        }

        // Tidy up and return final object
        delete[] rowData;
        png_destroy_read_struct(&png, &info, nullptr);
        bitmap.pixels.shrink_to_fit();
        bitmap.width = (scaleWidth > 0 ? scaleWidth : bitmap.width);
        bitmap.height = (scaleHeight > 0 ? scaleHeight : bitmap.height);
        return bitmap;
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