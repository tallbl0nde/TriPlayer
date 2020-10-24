#ifndef SOURCES_MP3_HPP
#define SOURCES_MP3_HPP

#include <array>
#include <string>
#include "sources/Source.hpp"

// Forward declaration as we only need the pointers here
typedef struct mpg123_handle_struct mpg123_handle;
namespace NX {
    class File;
};

// Extends Source to support MP3 files
// This class is not thread-safe!
class MP3 : public Source {
    private:
        // mpg123 instance
        static mpg123_handle * mpg;

        // Object associated with file
        NX::File * file;

        // Logs most recent error
        static void logErrorMsg();

    public:
        // Takes path to a .mp3 file
        MP3(const std::string &);

        size_t decode(unsigned char *, size_t);
        void seek(size_t);
        size_t tell();

        // Closes associated file
        ~MP3();

        // Initialize mpg123
        static bool initLib();
        // Cleanup mpg123
        static void freeLib();

        // Set seek method
        static bool setAccurateSeek(const bool);
        // Set the equalizer for decoding
        static bool setEqualizer(const std::array<float, 32> &);
};

#endif