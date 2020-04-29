#ifndef MP3_HPP
#define MP3_HPP

#include <mpg123.h>
#include "Source.hpp"
#include <string>

// Extends Source to support MP3 files
class MP3 : public Source {
    private:
        // mpg123 instance
        mpg123_handle * mpg;

    public:
        // Initialize mpg123 library
        static bool initLib();

        // Takes path to a .mp3 file
        MP3(std::string);

        virtual size_t decode(unsigned char *, size_t);

        // Closes file and frees mpg123
        ~MP3();
};

#endif