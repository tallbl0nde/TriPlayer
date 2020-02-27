#ifndef TYPES_HPP
#define TYPES_HPP

// A songID is an int!
typedef int SongID;

// Struct storing information about song
struct SongInfo {
    SongID ID;              // unique ID
    std::string title;      // title
    std::string artist;     // artist name
    std::string album;      // album name
    unsigned int duration;  // in seconds
};

#endif