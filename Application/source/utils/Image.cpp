#include "avir.h"
#include <cstdio>
#include <cstring>
#include <jpeglib.h>
#include "Log.hpp"
#include <png.h>
#include "utils/Image.hpp"

namespace Utils::Image {
    // Type of image
    enum class ImageFormat {
        PNG,        // Image is a PNG
        JPEG,       // Image is either JPEG, JFIF or EXIF (we don't care which one)
        Unknown     // Couldn't determine type of image
    };

    // Collection of data needed to identify an image
    struct ImageData {
        std::vector<uint8_t> pixels;    // Individual pixel data
        unsigned int width;             // Width of image in pixels
        unsigned int height;            // Height of image in pixels
        int bitDepth;                   // Number of bits per
        size_t channels;                // Number of colour channels
    };

    // Wraps a vector to provide 'stream'-like functionality
    struct VectorStream {
        std::vector<unsigned char> & buffer;    // Vector
        size_t pos;                             // Position of next byte to read
    };

    // Return type of image in buffer
    ImageFormat getImageFormat(std::vector<unsigned char> & data) {
        if (data.size() < 8) {
            return ImageFormat::Unknown;
        }

        // Check if PNG first
        if (!png_sig_cmp(&data[0], 0, 8)) {
            return ImageFormat::PNG;
        }

        // Check if JPEG otherwise
        if (data[0] == 0xFF && data[1] == 0xD8 && data[data.size() - 2] == 0xFF && data[data.size() - 1] == 0xD9) {
            return ImageFormat::JPEG;
        }

        // Otherwise we dunno what it actually is
        Log::writeError("[IMAGE] Failed to determine type of image");
        return ImageFormat::Unknown;
    }

    // Extracts the raw pixel values from the given JPEG buffer
    ImageData extractJPEG(std::vector<unsigned char> & data) {
        ImageData extracted;
        extracted.width = 0;
        extracted.height = 0;
        extracted.channels = 0;

        // Initialize library
        jpeg_decompress_struct jpeg;
        jpeg_error_mgr err;
        jpeg.err = jpeg_std_error(&err);
        jpeg_create_decompress(&jpeg);

        // Parse and decrypt jpeg
        jpeg_mem_src(&jpeg, &data[0], data.size());
        int result = jpeg_read_header(&jpeg, true);
        if (result != 1) {
            // Invalid JPEG
            Log::writeError("[IMAGE] JPEG is corrupt or invalid");
            return extracted;
        }
        jpeg_start_decompress(&jpeg);

        // Set metadata
        extracted.width = jpeg.output_width;
        extracted.height = jpeg.output_height;
        extracted.bitDepth = 8;
        extracted.channels = jpeg.output_components;

        // Get pixel data
        size_t bytesPerRow = extracted.width * extracted.channels;
        extracted.pixels.resize(extracted.height * bytesPerRow);
        while (jpeg.output_scanline < jpeg.output_height) {
            uint8_t * tmp = &extracted.pixels[jpeg.output_scanline * bytesPerRow];
            jpeg_read_scanlines(&jpeg, &tmp, 1);
        }

        // Clean up and return
        jpeg_finish_decompress(&jpeg);
        jpeg_destroy_decompress(&jpeg);
        return extracted;
    }

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

    // Callback for libpng to write to buffer
    void writeDataToBuffer(png_structp png, png_bytep bytes, png_size_t amount) {
        // Get pointer to vector
        png_voidp io_ptr = png_get_io_ptr(png);
        if (io_ptr == nullptr) {
            return;
        }
        VectorStream * data = (VectorStream *)io_ptr;

        // Append bytes to vector
        data->buffer.resize(data->pos + amount, 0);
        std::memcpy(&data->buffer[data->pos], bytes, amount);
        data->pos += amount;
    }

    void compressPNGPixels(ImageData & image, const png_structp & png, const png_infop & info) {
        // Get number of bytes per row
        size_t bytesPerRow = image.width * image.channels;

        // Iterate over each row of pixels
        for (size_t row = 0; row < image.height; row++) {
            // Write each row
            png_write_row(png, &image.pixels[row * bytesPerRow]);
        }
    }

    // Converts a raw image to PNG
    std::vector<unsigned char> compressPNG(ImageData image) {
        // We're going to assume the passed image is correct
        std::vector<unsigned char> compressed;

        // Wrap output data as a 'stream'
        VectorStream output = {compressed, 0};

        // Initialize library
        png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (png == nullptr) {
            Log::writeError("[IMAGE] Error initializing PNG library");
            return compressed;
        }
        png_infop info = png_create_info_struct(png);
        if (info == nullptr) {
            png_destroy_write_struct(&png, nullptr);
            Log::writeError("[IMAGE] Error initializing PNG library");
            return compressed;
        }
        png_set_write_fn(png, (void *) &output, writeDataToBuffer, nullptr);

        // Write header
        png_set_IHDR(png, info, image.width, image.height, image.bitDepth, (image.channels == 3 ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA), PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png, info);

        // Write pixel data
        compressPNGPixels(image, png, info);
        png_write_end(png, info);
        return compressed;
    }

    void extractPNGPixels(ImageData & image, const png_structp & png, const png_infop & info) {
        // Create buffer for row
        png_uint_32 bytesPerRow = png_get_rowbytes(png, info);
        uint8_t * rowData = new uint8_t[bytesPerRow];

        // Iterate over each row of pixels
        for (png_uint_32 row = 0; row < image.height; row++) {
            // Read the pixels
            png_read_row(png, (png_bytep)rowData, nullptr);

            // Append row to end of buffer
            image.pixels.resize(image.pixels.size() + bytesPerRow, 0);
            std::memcpy(&image.pixels[row * bytesPerRow], rowData, bytesPerRow);
        }

        delete[] rowData;
    }

    // Extracts the raw pixel values from the given PNG buffer
    ImageData extractPNG(std::vector<unsigned char> & data) {
        ImageData extracted;
        extracted.width = 0;
        extracted.height = 0;
        extracted.channels = 0;

        // Wrap input data as a 'stream'
        VectorStream input = {data, 0};

        // Sanity check
        if (png_sig_cmp(&data[0], 0, 8)) {
            return extracted;
        }

        // Initialize library
        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (png == nullptr) {
            Log::writeError("[IMAGE] Error initializing PNG library");
            return extracted;
        }
        png_infop info = png_create_info_struct(png);
        if (info == nullptr) {
            png_destroy_read_struct(&png, nullptr, nullptr);
            Log::writeError("[IMAGE] Error initializing PNG library");
            return extracted;
        }
        png_set_read_fn(png, (void *) &input, readDataFromBuffer);

        // Read PNG header to extract important info
        png_read_info(png, info);
        int colourType;
        png_uint_32 rc = png_get_IHDR(png, info, &extracted.width, &extracted.height, &extracted.bitDepth, &colourType, nullptr, nullptr, nullptr);
        if (rc != 1) {
            png_destroy_read_struct(&png, &info, nullptr);
            Log::writeError("[IMAGE] Failed to read PNG header");
            return extracted;
        }

        // Now read the image
        switch (colourType) {
            case PNG_COLOR_TYPE_RGB:
                extracted.channels = 3;
                break;

            case PNG_COLOR_TYPE_RGB_ALPHA:
                extracted.channels = 4;
                break;

            // We don't support other types
            default:
                Log::writeError("[IMAGE] Unsupported PNG pixel type");
        }
        if (extracted.channels > 0) {
            extractPNGPixels(extracted, png, info);
        }

        // Tidy up and return
        png_destroy_read_struct(&png, &info, nullptr);
        return extracted;
    }

    bool resize(std::vector<unsigned char> & data, size_t destW, size_t destH) {
        // Determine the format of the image first and then extract raw pixel data
        ImageFormat format = getImageFormat(data);
        ImageData image;
        switch (format) {
            case ImageFormat::PNG:
                image = extractPNG(data);
                break;

            case ImageFormat::JPEG:
                image = extractJPEG(data);
                break;

            default:
                Log::writeError("[IMAGE] Couldn't determine image format");
                return false;
                break;
        }

        // Stop if we have no pixels
        if (image.pixels.empty()) {
            Log::writeInfo("[IMAGE] Extracted zero pixels from image; can't resize");
            return false;
        }

        // Resize if it's not the destination size
        if (!(image.width == destW && image.height == destH)) {
            // Create the output buffer
            std::vector<uint8_t> resized;
            resized.resize(destW * destH * image.channels, 0);

            // Resize and fill buffer
            avir::CImageResizer<> ImageResizer(8);
            ImageResizer.resizeImage(&image.pixels[0], image.width, image.height, 0, &resized[0], destW, destH, image.channels, 0);

            // Update ImageData struct
            image.pixels = resized;
            image.width = destW;
            image.height = destH;

        } else {
            Log::writeInfo("[IMAGE] No need to resize image as it has the required dimensions");
        }

        // We're going to save a PNG cause they're better than JPEG
        data = compressPNG(image);
        data.shrink_to_fit();
        Log::writeInfo("[IMAGE] Resized successfully");
        return true;
    }
};