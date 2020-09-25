#ifndef IPC_COMMAND_HPP
#define IPC_COMMAND_HPP

// This file contain the IDs of susmodule commands, along with a description of what they do
// Note that the client can only send commands to the sysmodule, not the other way around!
namespace Ipc {
    // Commands:            // WHAT IT DOES                                     // WHAT THE CLIENT SENDS WITH COMMAND               // WHAT THE SYSMODULE REPLIES WITH (APART FROM RESULT)
    enum class Command {
        Version,            // Request version of sysmodule (always id 0)       // Nothing                                          // Version of sysmodule (string)
        Resume,             // Resume playback                                  // Nothing                                          // Nothing
        Pause,              // Pause playback                                   // Nothing                                          // Nothing
        Previous,           // Jump to last played song                         // Nothing                                          // Nothing
        Next,               // Skip to next song in queue                       // Nothing                                          // Nothing
        GetVolume,          // Get volume level                                 // Nothing                                          // Volume level [double (between 0.0 and 100.0)]
        SetVolume,          // Set volume level                                 // New volume level (between 0.0 and 100.0)         // New volume level [double (between 0.0 and 100.0)]
        Mute,               // Mute music                                       // Nothing                                          // Nothing
        Unmute,             // Unmute music (if muted)                          // Nothing                                          // New volume level [double (between 0.0 and 100.0)]
        GetSubQueue,        // Get sub-queue                                    // First index and number to get                    // Sequence of IDs matching sub-queue
        SkipSubQueueSongs,  // Skip forward given number of songs + play        // Number of songs to skip                          // Number of songs skipped
        SubQueueSize,       // Get number of songs in sub-queue                 // Nothing                                          // Number of songs in sub-queue
        AddToSubQueue,      // Add song to 'sub-queue'                          // ID of song to add to 'sub-queue'                 // Nothing
        RemoveFromSubQueue, // Remove song from 'sub-queue'                     // Position of song to remove                       // Nothing
        QueueIdx,           // Get position of current song in queue            // Nothing                                          // Position of currently playing song in queue
        SetQueueIdx,        // Set index of current song in queue               // Position to move to                              // The new queue index
        QueueSize,          // Get number of songs in queue                     // Nothing                                          // Number of songs in queue
        RemoveFromQueue,    // Remove song from queue                           // Position of song to remove                       // Nothing
        GetQueue,           // Get play queue                                   // First index and number to get                    // Sequence of IDs matching queue
        SetQueue,           // Set play queue songs (will clear)                // Sequence of IDs to add to queue                  // Number of songs added to queue
        GetRepeat,          // Return repeat mode                               // Nothing                                          // Repeat matching state
        SetRepeat,          // Set repeat mode                                  // Repeat mode to set                               // Nothing
        GetShuffle,         // Return status of shuffle                         // Nothing                                          // Shuffle matching state
        SetShuffle,         // (Un)shuffle                                      // Shuffle mode to set                              // Nothing
        GetSong,            // Get the ID of currently playing song             // Nothing                                          // ID of song playing (negative if no song playing!)
        GetStatus,          // Get the status of the sysmodule                  // Nothing                                          // Status matching state
        GetPosition,        // Return percentage of song played                 // Nothing                                          // Percentage of song played [double (between 0.0 and 100.0)]
        SetPosition,        // Seeks to a spot in the song                      // Percentage to seek to [double (0.0 - 100.0)]     // Percentage seeked to [double (between 0.0 and 100.0)]
        GetPlayingFrom,     // Returns text saying what's in the queue          // Nothing                                          // 'Playing from' string
        SetPlayingFrom,     // Set 'playing from' text (allows 100 chars)       // String to set                                    // Set string (maybe be substring if too long)
        RequestDBLock,      // Requests exclusive access to DB (blocks)         // Nothing                                          // Nothing
        ReleaseDBLock,      // Releases exclusive access to DB                  // Nothing                                          // Nothing
        ReloadConfig,       // Get the sysmodule to update it's config          // Nothing                                          // Nothing
        Reset               // Reinitialize sysmodule (except ipc service)      // Nothing                                          // Version of sysmodule (string)
    };

    // Shuffle on?
    enum class Shuffle {
        Off,                // Queue not shuffled
        On                  // Queue shuffled
    };

    // Repeat mode
    enum class Repeat {
        Off,                // Don't repeat
        One,                // Repeat the same song
        All                 // Repeat the queue
    };

    // Sysmodule state
    enum class Status {
        Error,              // Error occurred getting status
        Playing,            // Playing song
        Paused,             // Song(s) queued but paused
        Stopped             // No songs queued
    };
};

#endif