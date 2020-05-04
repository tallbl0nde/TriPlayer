#ifndef TYPES_HPP
#define TYPES_HPP

enum class AudioStatus {
    Playing,    // Buffer is being played
    Paused,     // Buffer is queued but paused
    Stopped     // No buffer is playing/queued
};

enum class RepeatMode {
    Off,        // Don't repeat
    One,        // Repeat the same song
    All         // Repeat the queue
};

typedef int SongID;

#endif