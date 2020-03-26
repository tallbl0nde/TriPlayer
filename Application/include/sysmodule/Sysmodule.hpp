#ifndef SYSMODULE_HPP
#define SYSMODULE_HPP

#include <atomic>
#include <mutex>
#include <queue>
#include "Socket.hpp"
#include "Types.hpp"
#include <vector>

// The sysmodule class makes use of a socket to communicate with the
// sysmodule.
class Sysmodule {
    private:
        // Socket file descriptor (set negative if an error occurred)
        SockFD socket;

        // Status vars
        std::atomic<SongID> currentID;
        std::atomic<double> position_;
        std::atomic<PlaybackStatus> status_;
        // queue here too
        std::atomic<int> version;   // Set negative if an error occurred
        std::atomic<int> volume;

        // Writes are queued
        void addToWQueue(std::string);
        // Mutex to protect pushes onto queue
        std::mutex queueMutex;
        // Socket write queue
        std::queue<std::string> writeQueue;

        // Functions which actually update status vars
        bool getPosition_();
        bool getSongID_();
        bool getStatus_();

    public:
        // Constructor creates a socket and attempts connection to sysmodule
        Sysmodule();
        // Returns whether versions match (will return false if the connection failed as well)
        bool isConnected();
        // (Drops) current socket and reconnects
        void reconnect();

        // Function to poll sysmodule frequently
        void updateState();

        // Sends relevant command to sysmodule, returning true if the sysmodule received the command
        // Playback commands
        void resumePlayback();
        void pausePlayback();
        void previousSong();
        void nextSong();
        void setVolume(int); // 0 to 100

        // Manipulate queue
        void playSong(SongID);
        void addToQueue(SongID);
        void removeFromQueue(SongID);
        // bool getQueue(std::vector<SongID> &); // Vector to store IDs in
        // bool setQueue(std::vector<SongID>); // Vector of IDs to add

        // Shuffle current queue
        void shuffleQueue();

        // Toggle repeat
        void repeatOn();
        void repeatOff();

        // Status
        SongID playingID(); // Variable to store ID in
        double position(); // Variable to store percentage played (0 - 100)
        PlaybackStatus status(); // Variable to store status in
        // get shuffle get repeat

        // Reinit sysmodule - will cause socket to disconnect!
        bool reset();

        // Call to 'join' thread (stops updateState loop)
        void finish();

        // Destructor closes socket
        ~Sysmodule();
};

#endif