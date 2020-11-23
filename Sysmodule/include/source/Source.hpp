#ifndef SOURCE_SOURCE_HPP
#define SOURCE_SOURCE_HPP

#include <cstddef>

// Forward declarations
enum class Format;

// A Source is an abstract class representing an audio source.
// Codec specific class will inherit this an implement required behaviour.
namespace Source {
    class Source {
        protected:
            // These must be set by children
            int channels_;
            bool done_;
            Format format_;
            long sampleRate_;
            int totalSamples_;
            bool valid_;

        public:
            Source();

            // Decode audio into buffer with limiting size
            // Returns number of bytes decoded
            virtual size_t decode(unsigned char *, size_t) = 0;

            // Returns true when all data has been decoded
            bool done();
            // Returns true if file was opened without errors
            bool valid();

            // Seek to sample in song
            virtual void seek(size_t) = 0;
            // Return position in song (in samples)
            virtual size_t tell() = 0;

            // Return number of channels
            int channels();
            // Return format of decoded samples
            Format format();
            // Returns sample rate
            long sampleRate();
            // Returns total number of samples
            int totalSamples();

            virtual ~Source();
    };
};

#endif