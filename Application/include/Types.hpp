#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>

// A songID is an int!
typedef int SongID;
// A socket is also an int - this is for easier reading
typedef int SockFD;

// Struct storing information about song
struct SongInfo {
    SongID ID;              // unique ID
    std::string title;      // title
    std::string artist;     // artist name
    std::string album;      // album name
    unsigned int duration;  // in seconds
};

// SysmoduleStatuses
enum SysmoduleStatus {
    ERROR,      // Error occurred getting status
    PLAYING,    // Playing song
    PAUSED,     // Song(s) queued but paused
    STOPPED     // No songs queued
};

#endif