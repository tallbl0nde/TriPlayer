#ifndef SYSMODULE_HPP
#define SYSMODULE_HPP

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <queue>
#include "Types.hpp"
#include <vector>

// The sysmodule class communicates with the sysmodule via IPC calls.
// All values are cached, allowing for fast access and no race conditions!
class Sysmodule {
    public:
        // Reasons for loss of communication
        enum class Error {
            None,                   // No error
            DifferentVersion,       // App and sysmodule versions don't match
            NotConnected,           // Couldn't establish a connection
            Unknown                 // Unknown error
        };

    private:
        std::atomic<bool> connected_;
        std::atomic<Error> error_;
        std::atomic<bool> exit_;
        std::atomic<int> limit_;
        std::chrono::steady_clock::time_point lastUpdateTime;

        // === Status vars ===
        std::atomic<SongID> currentSong_;
        std::mutex playingFromMutex;
        std::string playingFrom_;
        std::atomic<double> position_;
        std::atomic<bool> queueChanged_;        // Set true when the whole queue has been updated (not just a single song)
        std::vector<SongID> queue_;
        std::mutex queueMutex;
        std::atomic<size_t> queueSize_;
        std::atomic<RepeatMode> repeatMode_;
        std::atomic<ShuffleMode> shuffleMode_;
        std::atomic<bool> subQueueChanged_;     // Set true when the whole queue has been updated (not just a single song)
        std::vector<SongID> subQueue_;
        std::mutex subQueueMutex;
        std::atomic<size_t> subQueueSize_;
        std::atomic<size_t> songIdx_;
        std::atomic<PlaybackStatus> status_;
        std::atomic<double> volume_;
        // ======

        // Queue of IPC commands
        std::queue< std::function<bool()> > ipcQueue;
        std::mutex ipcMutex;

        // Returns if the message was added to the queue
        bool addToIpcQueue(std::function<bool()>);

    public:
        // Constructor creates a socket and attempts connection to sysmodule
        Sysmodule();

        // Returns type of error occurred
        Error error();
        // Uses syscalls to try to launch the sysmodule
        bool launch();
        // Drops current connection if there is one and attempts to reconnect
        void reconnect();
        // Uses syscalls to try and terminate the sysmodule
        bool terminate();
        // Set a limit on the number of songs to queue (negative indicates no limit)
        void setQueueLimit(int);

        // Main function which updates state when replies received
        void process();

        // === Returns private (state) variables ===
        SongID currentSong();
        std::string playingFrom();
        double position();
        bool queueChanged();
        std::vector<SongID> queue();
        size_t queueSize();
        RepeatMode repeatMode();
        ShuffleMode shuffleMode();
        size_t songIdx();
        bool subQueueChanged();
        std::vector<SongID> subQueue();
        size_t subQueueSize();
        PlaybackStatus status();
        double volume();

        // The following commands block the calling thread until a response is received
        bool waitRequestDBLock();
        bool waitReset();
        size_t waitSongIdx();

        // === Send command to sysmodule ===
        // Updates relevant variable when reply received or sets error() true
        // See Common/Protocol.hpp for explanation of functions

        // Playback commands
        void sendResume();
        void sendPause();
        void sendPrevious();
        void sendNext();

        void sendGetVolume();
        void sendSetVolume(const double);

        void sendMute();
        void sendUnmute();

        // Manipulate queue
        void sendGetSubQueue();
        void sendGetSubQueueSize();

        void sendAddToSubQueue(const SongID);
        void sendRemoveFromSubQueue(const size_t);
        void sendSkipSubQueueSongs(const size_t);

        void sendGetQueue();
        void sendGetQueueSize();
        void sendSetQueue(const std::vector<SongID> &);

        void sendGetSongIdx();
        void sendSetSongIdx(const size_t);
        void sendRemoveFromQueue(const size_t);

        // Shuffle/repeat
        void sendGetRepeat();
        void sendSetRepeat(const RepeatMode);

        void sendGetShuffle();
        void sendSetShuffle(const ShuffleMode);

        // Status
        void sendGetSong();
        void sendGetStatus();

        void sendGetPosition();
        void sendSetPosition(double);

        void sendGetPlayingFrom();
        void sendSetPlayingFrom(const std::string &);

        void sendReleaseDBLock();
        void sendReloadConfig();

        // Call to 'join' thread (stops main loop)
        void exit();

        // Destructor closes socket
        ~Sysmodule();
};

#endif