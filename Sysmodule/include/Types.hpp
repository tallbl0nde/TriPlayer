#ifndef TYPES_HPP
#define TYPES_HPP

enum class AudioStatus {
    Playing,    // Buffer is being played
    Paused,     // Buffer is queued but paused
    Stopped     // No buffer is playing/queued
};

typedef int SongID;

#endif