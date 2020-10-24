#ifndef IPC_TRIPLAYER_HPP
#define IPC_TRIPLAYER_HPP

#include <string>
#include <vector>

// This file contains all the definitions + descriptions of functions
// used to interface with TriPlayer's sysmodule. If you're reading this
// and are wondering how to use this in your project, simply copy this
// and the relevant .cpp and use in a similar fashion to libnx calls :)
//
// Note: All functions return true if successful, or false on an error!
namespace TriPlayer {
    // Repeat mode
    enum class Repeat {
        Off,        // Don't repeat any songs or the queue
        One,        // Repeat one song
        All         // Repeat the play queue
    };

    // Shuffle mode
    enum class Shuffle {
        Off,        // Queue is not shuffled
        On          // Queue is shuffled
    };

    // Sysmodule status
    enum class Status {
        Playing,    // Playing a track
        Paused,     // Track is paused
        Stopped,    // No tracks queued
        Error       // A fatal error occurred
    };

    // Initialize and connect to the sysmodule
    // Common reasons of failure are either it's not running or there's a version mismatch
    bool initialize();
    // Disconnect and tidy up the session
    void exit();

    // Get the sysmodule's version (encoded as a string with format X.X.X)
    bool getVersion(std::string & outVersion);

    // Control the playback state
    bool resume();
    bool pause();
    bool previous();
    bool next();

    // Get the current volume level (ranges from 0.0 to 100.0)
    bool getVolume(double & outVolume);
    // Set the volume for playback (values outside of 0.0 to 100.0 will be capped)
    bool setVolume(const double volume);

    // Mute the audio volume
    bool mute();
    // Unmute volume and get new volume
    bool unmute(double & outVolume);

    // Get a list of song IDs in the 'sub-queue'
    // The 'sub-queue' contains songs manually added by the user
    bool getSubQueue(std::vector<int> & outIDs);
    // Get the number of songs in the sub-queue
    bool getSubQueueSize(size_t & outCount);

    // Add a song ID to the sub-queue
    bool addToSubQueue(const int ID);
    // Remove a song from the given index in the sub-queue
    bool removeFromSubQueue(const size_t pos);
    // Skip a number of songs in the sub-queue and play
    bool skipSubQueueSongs(const size_t count);

    // Get a list of song IDs in the main queue
    // The main queue is set when playing an album, playlist, etc
    bool getQueue(std::vector<int> & outIDs);
    // Get the number of songs in the main queue
    bool getQueueSize(size_t & outCount);
    // Set the IDs in the main queue
    bool setQueue(const std::vector<int> & IDs);

    // Get the current position of the current track in the main queue
    bool getQueueIdx(size_t & outPos);
    // Play the track at the given index in the main queue
    bool setQueueIdx(const size_t pos);
    // Remove the track at the given index from the queue
    bool removeFromQueue(const size_t pos);

    // Get the TriPlayer::Repeat mode of the sysmodule
    bool getRepeatMode(Repeat & outMode);
    // Set the TriPlayer::Repeat mode
    bool setRepeatMode(const Repeat mode);

    // Get the TriPlayer::Shuffle mode of the sysmodule
    bool getShuffleMode(Shuffle & outMode);
    // Set the TriPlayer::Shuffle mode
    bool setShuffleMode(const Shuffle mode);

    // Get the currently playing song's ID
    bool getSongID(int & outID);
    // Get the TriPlayer::Status of the sysmodule
    bool getStatus(Status & outStatus);

    // Get the position in the current song (ranges from 0.0 to 100.0)
    bool getPosition(double & outPos);
    // Jump to the position in the song (values outside of 0.0 to 100.0 will be capped)
    bool setPosition(const double pos);

    // Get the 'playback source' of the current queue
    bool getPlayingFromText(std::string & outText);
    // Set the 'playback source' for the queue
    bool setPlayingFromText(const std::string & text);

    // Request exclusive access to the database file
    // This may block for a while depending on what the sysmodule is doing
    bool requestDatabaseLock();
    // Release previously requested access to database
    bool releaseDatabaseLock();

    // Request the sysmodule to re-read it's config file
    bool reloadConfig();
    // Reset everything but the IPC connection
    bool reset();
    // Safely terminate the sysmodule, freeing the IPC service
    bool stopSysmodule();
};

#endif