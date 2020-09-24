#ifndef TYPES_HPP
#define TYPES_HPP

namespace Tri {
    enum class Result {
        Ok
    };
};

enum class RepeatMode {
    Off,        // Don't repeat
    One,        // Repeat the same song
    All         // Repeat the queue
};

typedef int SongID;

#endif