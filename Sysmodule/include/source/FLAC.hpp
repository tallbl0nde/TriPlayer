#ifndef SOURCE_FLAC_HPP
#define SOURCE_FLAC_HPP

#include "source/Source.hpp"

// Forward declaration as only the pointer is needed here
struct dr_flac;

// Extends Source to support FLAC files
// This class is not thread-safe!
namespace Source {
    class FLAC : public Source {
        private:
            // FLAC decoder
            dr_flac * flac;

        public:
            // Constructor takes path to FLAC file
            FLAC(const std::string &);

            size_t decode(unsigned char *, size_t);
            void seek(size_t);
            size_t tell();

            // Closes associated file
            ~FLAC();
    };
};

#endif