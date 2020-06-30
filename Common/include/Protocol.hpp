#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

// This file stores everything related to the protocol used for communication between sysmodule and clients.
// Note that the client can only send commands to the sysmodule, not the other way around!
namespace Protocol {
    // Character used to separate strings (record separator)
    extern const char Delimiter;
    // Port sysmodule listens on
    extern const int Port;
    // Number of seconds to wait for reply before timing out
    extern const int Timeout;
    // Protocol version (will always be only byte returned when VERSION requested)
    extern const int Version;

    // Commands:            // WHAT IT DOES                               // WHAT THE CLIENT SENDS AFTER COMMAND              // WHAT THE SYSMODULE REPLIES WITH
    enum class Command {
        Version,            // Request version of sysmodule               // Nothing                                          // Version of sysmodule
        Resume,             // Resume playback                            // Nothing                                          // ID of song playing
        Pause,              // Pause playback                             // Nothing                                          // ID of song playing
        Previous,           // Jump to last played song                   // Nothing                                          // Result code (0 = success)
        Next,               // Skip to next song in queue                 // Nothing                                          // Result code (0 = success)
        GetVolume,          // Get volume level                           // Nothing                                          // Volume level [double (between 0.0 and 100.0)]
        SetVolume,          // Set volume level                           // New volume level (between 0.0 and 100.0)         // New volume level [double (between 0.0 and 100.0)]
        Mute,               // Mute music                                 // Nothing                                          // New volume level (always 0)
        Unmute,             // Unmute music (if muted)                    // Nothing                                          // New volume level [double (between 0.0 and 100.0)]
        GetSubQueue,        // Get sub-queue                              // First index and number to get                    // Delimited sequence of IDs matching sub-queue
        SkipSubQueueSongs,  // Skip forward given number of songs + play  // Number of songs to skip                          // Number of songs skipped
        SubQueueSize,       // Get number of songs in sub-queue           // Nothing                                          // Number of songs in sub-queue
        AddToSubQueue,      // Add song to 'sub-queue'                    // ID of song to add to 'sub-queue'                 // ID of song added
        RemoveFromSubQueue, // Remove song from 'sub-queue'               // Position of song to remove                       // Position of song removed
        QueueIdx,           // Get position of current song in queue      // Nothing                                          // Position of currently playing song in queue
        SetQueueIdx,        // Set index of current song in queue         // Position to move to                              // The new queue index
        QueueSize,          // Get number of songs in queue               // Nothing                                          // Number of songs in queue
        RemoveFromQueue,    // Remove song from queue                     // Position of song to remove                       // Position of song removed
        GetQueue,           // Get play queue                             // First index and number to get                    // Delimited sequence of IDs matching queue
        SetQueue,           // Set play queue songs (will clear)          // Delimited sequence of IDs to add to queue        // Number of songs added to queue
        GetRepeat,          // Return repeat mode                         // Nothing                                          // Repeat matching state
        SetRepeat,          // Set repeat mode                            // SM_Repeat mode to set                            // Repeat matching state
        GetShuffle,         // Return status of shuffle                   // Nothing                                          // Shuffle matching state
        SetShuffle,         // (Un)shuffle                                // SM_Shuffle mode to set                           // Shuffle matching state
        GetSong,            // Get the ID of currently playing song       // Nothing                                          // ID of song playing (negative if no song playing!)
        GetStatus,          // Get the status of the sysmodule            // Nothing                                          // Status matching state
        GetPosition,        // Return percentage of song played           // Nothing                                          // Percentage of song played [double (between 0.0 and 100.0)]
        SetPosition,        // Seeks to a spot in the song                // Percentage to seek to [double (0.0 - 100.0)]     // Percentage seeked to [double (between 0.0 and 100.0)]
        Reset               // Reinitialize sysmodule (except socket)     // Nothing                                          // Version of sysmodule
    };

    // Shuffle on?
    enum class Shuffle {
        Off,        // Queue shuffled
        On          // Queue not shuffled
    };

    // Repeat mode
    enum class Repeat {
        Off,        // Don't repeat
        One,        // Repeat the same song
        All         // Repeat the queue
    };

    // Sysmodule state
    enum class Status {
        Error,      // Error occurred getting status
        Playing,    // Playing song
        Paused,     // Song(s) queued but paused
        Stopped     // No songs queued
    };
};

#endif