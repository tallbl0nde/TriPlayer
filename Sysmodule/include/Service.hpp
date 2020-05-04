#ifndef MAINSERVICE_HPP
#define MAINSERVICE_HPP

#include <atomic>
#include "Audio.hpp"
#include <ctime>
#include "Database.hpp"
#include "PlayQueue.hpp"
#include <mutex>
#include "Socket.hpp"
#include "Source.hpp"

// Class which manages all actions taken when receiving a command
// Essentially encapsulates everything
class MainService {
    private:
        // Audio instance
        Audio * audio;
        // Database object
        Database * db;
        // Queue of songs
        PlayQueue * queue;
        // Socket object for communication
        Socket * socket;

        // Whether to stop loop and exit
        std::atomic<bool> exit_;
        // Timestamp of last previous press
        std::time_t pressTime;
        // Repeat mode
        std::atomic<RepeatMode> repeatMode;

        // Mutex for accessing source
        std::mutex sMutex;
        // Source currently playing
        Source * source;
        // Set true to stop decode thread waiting for a free buffer
        std::atomic<bool> skip;

    public:
        // Constructor initializes socket related things
        MainService();

        // Call to exit and prepare for deletion (will stop loop)
        void exit();

        // Function to run which listens and takes action when receiving a command
        void process();

        // TEMP (shouldn't be part of this class)
        void decodeSource();

        ~MainService();
};

#endif