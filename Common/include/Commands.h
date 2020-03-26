#ifndef SYSMODULE_COMMANDS_H
#define SYSMODULE_COMMANDS_H

// This file stores the code for each command sent/received due to communication
// with the sysmodule.

// Protocol version
extern const int SM_PROTOCOL_VERSION;

// Commands
enum SM_Command {
    VERSION,            // Version command will always have first byte set to 0x0
    RESUME,             // Resume playback if a song is playing
    PAUSE,              // Pause playback
    PREVIOUS,           // Jump to last played song
    NEXT,               // Skip to next song in queue
    GETVOLUME,          // Get the volume level (double between 0 and 100)
    SETVOLUME,          // Set the volume level (double between 0 and 100)
    PLAY,               // Play the song with given ID
    ADDTOQUEUE,         // Add the ID to queue
    REMOVEFROMQUEUE,    // Remove song from queue with position
    GETQUEUE,           // Get a list of IDs in the queue
    SETQUEUE,           // Set a list of IDs as the queue
    SHUFFLE,            // Toggles shuffle
    SETREPEAT,          // Set repeat mode (see SM_Repeat below)
    GETSONG,            // Get the ID of currently playing song (undefined if no song playing!)
    GETSTATUS,          // Get the status of the sysmodule (see SM_Status below)
    GETPOSITION,        // Return percentage of song played
    GETSHUFFLE,         // Return status of shuffle
    GETREPEAT,          // Return status of repeat
    RESET               // Reinitialize the sysmodule (as if killed and launched)
};

// Repeat mode
enum SM_Repeat {
    OFF,        // Don't repeat
    ONE,        // Repeat the same song
    ALL         // Repeat the queue
};

// Sysmodule state
enum SM_Status {
    ERROR,      // Error occurred getting status
    PLAYING,    // Playing song
    PAUSED,     // Song(s) queued but paused
    STOPPED     // No songs queued
};

// Character used to separate strings (record separator)
extern const char SM_DELIMITER;

#endif