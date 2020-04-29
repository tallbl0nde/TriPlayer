#ifndef SOURCE_HPP
#define SOURCE_HPP

#include <cstddef>

// A Source is an abstract class representing an audio source.
// Codec specific class will inherit this an implement required behaviour.
class Source {
    protected:
        // These must be set by children
        int channels_;
        long sampleRate_;
        int decodedSamples_;
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

        // Return position in song (0 - 100.0)
        double position();
        // Seek to position in song (0 - 100.0)
        // virtual void seek(double) = 0;

        // Return number of channels
        int channels();
        // Returns sample rate
        long sampleRate();

        virtual ~Source();
};

#endif