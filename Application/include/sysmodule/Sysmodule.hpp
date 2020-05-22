#ifndef SYSMODULE_HPP
#define SYSMODULE_HPP

#include <atomic>
#include <chrono>
#include <functional>
#include <shared_mutex>
#include "PlayQueue.hpp"
#include <queue>
#include "Types.hpp"
#include <vector>

// The sysmodule class makes use of a socket to communicate with the
// sysmodule.
class Sysmodule {
    private:
        // Socket file descriptor (set negative if an error occurred)
        SockFD socket;

        std::atomic<bool> error_;
        std::atomic<bool> exit_;
        std::chrono::steady_clock::time_point lastUpdateTime;

        // === Status vars ===
        std::atomic<SongID> currentSong_;
        std::atomic<double> position_;
        std::atomic<bool> queueChanged_;    // Set true when the whole queue has been updated (not just a single song)
        std::vector<SongID> queue_;
        std::shared_mutex queueMutex;
        std::atomic<size_t> queueSize_;
        std::atomic<RepeatMode> repeatMode_;
        std::atomic<ShuffleMode> shuffleMode_;
        std::atomic<size_t> songIdx_;
        std::atomic<PlaybackStatus> status_;
        std::atomic<double> volume_;
        // ======

        // Queue of messages to write and callback
        std::queue< std::pair<std::string, std::function<void(std::string)> > > writeQueue;
        std::shared_mutex writeMutex;
        void addToWriteQueue(std::string, std::function<void(std::string)>);

    public:
        // Constructor creates a socket and attempts connection to sysmodule
        Sysmodule();

        // Returns true when an error occurred in communication
        bool error();
        // (Drops) current socket and reconnects
        void reconnect();

        // Main function which updates state when replies received
        void process();

        // === Returns private (state) variables ===
        SongID currentSong();
        double position();
        bool queueChanged();
        std::vector<SongID> queue();
        size_t queueSize();
        RepeatMode repeatMode();
        ShuffleMode shuffleMode();
        size_t songIdx();
        PlaybackStatus status();
        double volume();

        // Blocks calling thread until it is guaranteed songIdx has been updated
        // Returns max size_t on error (e.g. not connected)
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

        // Manipulate queue
        void sendSetSongIdx(const size_t);
        void sendGetSongIdx();
        void sendGetQueueSize();
        void sendAddToQueue(const SongID);
        void sendRemoveFromQueue(const size_t);
        void sendGetQueue(const size_t, const size_t);
        void sendSetQueue(const std::vector<SongID> &);

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

        // Reinit sysmodule
        void sendReset();
        // ======

        // Call to 'join' thread (stops main loop)
        void exit();

        // Destructor closes socket
        ~Sysmodule();
};

#endif