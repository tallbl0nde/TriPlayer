#ifndef SOURCES_FLAC_HPP
#define SOURCES_FLAC_HPP

#include "sources/Source.hpp"

// Forward declaration as only the pointer is needed here
struct drflac;

// Extends Source to support FLAC files
// This class is not thread-safe!
class FLAC : public Source {
    private:
        // FLAC decoder
        drflac * flac;

    public:
        // Constructor takes path to FLAC file
        FLAC(const std::string &);

        size_t decode(unsigned char *, size_t);
        void seek(size_t);
        size_t tell();

        // Closes associated file
        ~FLAC();
};

#endif