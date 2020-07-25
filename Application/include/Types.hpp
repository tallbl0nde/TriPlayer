#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>

// All IDs are integers
typedef int ArtistID, AlbumID, PlaylistID, SongID;
// A socket is also an int - this is for easier reading
typedef int SockFD;

// Status of sysmodule playback
enum class PlaybackStatus {
    Error,      // An error occurred getting status
    Playing,    // Audio is being played
    Paused,     // Song is in middle of playback but is paused
    Stopped     // No song is playing/paused
};

// Repeat type
enum class RepeatMode {
    Off,
    One,
    All
};

// Is shuffle on?
enum class ShuffleMode {
    Off,
    On
};

// All strings are UTF-8 encoded
namespace Metadata {
    struct Album {
        AlbumID ID;                 // Album's unique ID
        std::string name;           // Album's name
        std::string artist;         // Artist's name
        int tadbID;                 // TheAudioDB ID of album (negative if not set)
        std::string imagePath;      // Path to album's image (can be blank)
        unsigned int songCount;     // Number of songs on album
    };

    struct Artist {
        ArtistID ID;                // Album's unique ID
        std::string name;           // Album's name
        int tadbID;                 // TheAudioDB ID of artist (negative if not set)
        std::string imagePath;      // Path to artist's image (can be blank)
        unsigned int albumCount;    // Number of albums
        unsigned int songCount;     // Number of songs
    };

    struct Playlist {
        PlaylistID ID;              // Playlist's unique ID
        std::string name;           // Playlist name
        std::string description;    // Playlist description (optional)
    };

    struct Song {
        SongID ID;                  // Song's unique ID (negative if an error occurred or not used)
        std::string title;          // Track title
        std::string artist;         // Artist name
        std::string album;          // Album name
        int trackNumber;            // Track number of song (0 if not set)
        int discNumber;             // Song's disc number on album
        unsigned int duration;      // Duration of track in seconds
        unsigned int plays;         // Number of plays (not used)
        bool favourite;             // Is the track favourited? (not used)
        std::string path;           // Path of associated file
        unsigned int modified;      // Timestamp file was last modified
    };
};

#endif