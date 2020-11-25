#ifndef SOURCE_WAV_HPP
#define SOURCE_WAV_HPP

#include "source/Source.hpp"

// Forward declaration as only the pointer is needed here
struct dr_wav;

// Extends Source to support WAV files
// This class is not thread-safe!
namespace Source {
    class WAV : public Source {
        private:
            // WAV decoder
            dr_wav * wav;

            // Size of one PCM frame (cached to prevent
            // unnecessary recalculations)
            size_t frameSize;

        public:
            // Constructor takes path to WAV file
            WAV(const std::string &);

            size_t decode(unsigned char *, size_t);
            void seek(size_t);
            size_t tell();

            // Closes associated file
            ~WAV();
    };
};

#endif