#ifndef SYSMODULE_HPP
#define SYSMODULE_HPP

#include "Socket.hpp"
#include "Types.hpp"
#include <vector>

// The sysmodule class makes use of a socket to communicate with the
// sysmodule.
class Sysmodule {
    private:
        // Socket file descriptor (set negative if an error occurred)
        SockFD socket;

    public:
        // Constructor creates a socket and attempts connection to sysmodule
        Sysmodule();
        // Returns whether versions match (will return false if the connection failed as well)
        bool isReady();

        // (Drops) current socket and reconnects
        void reconnect();

        // Sends relevant command to sysmodule, returning true if the sysmodule received the command
        // Playback commands
        bool resumePlayback();
        bool pausePlayback();
        bool previousSong();
        bool nextSong();

        // Alter volume
        bool decreaseVolume();
        bool increaseVolume();
        bool setVolume(int); // 0 to 100

        // Manipulate queue
        bool playSong(SongID);
        bool addToQueue(SongID);
        bool removeFromQueue(SongID);
        bool getQueue(std::vector<SongID> &); // Vector to store IDs in
        bool setQueue(std::vector<SongID>); // Vector of IDs to add

        // Shuffle current queue
        bool shuffleQueue();

        // Toggle repeat
        bool repeatOn();
        bool repeatOff();

        // Status
        bool getCurrentSong(SongID *); // Variable to store ID in
        bool getStatus(SysmoduleStatus *); // Variable to store status in

        // Reinit sysmodule - will cause socket to disconnect!
        bool reset();

        // Destructor closes socket
        ~Sysmodule();
};

#endif