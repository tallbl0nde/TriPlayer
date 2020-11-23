#ifndef TYPES_HPP
#define TYPES_HPP

// Format of decoded samples (values match libnx)
enum class Format {
    Int8 = 1,       // 8 bit integer
    Int16 = 2,      // 16 bit integer
    Int24 = 3,      // 24 bit integer
    Int32 = 4,      // 32 bit integer
    Float = 5       // (probably 32 bit) float
};

enum class RepeatMode {
    Off,        // Don't repeat
    One,        // Repeat the same song
    All         // Repeat the queue
};

typedef int SongID;

#endif