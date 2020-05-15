#ifndef MAINSERVICE_HPP
#define MAINSERVICE_HPP

#include <atomic>
#include "Audio.hpp"
#include <ctime>
#include "Database.hpp"
#include "PlayQueue.hpp"
#include "Protocol.hpp"
#include <shared_mutex>
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
        // Status vars for comm. between threads
        std::atomic<bool> songChanged;
        std::atomic<double> seekTo;

        // Mutex for accessing queue
        std::shared_mutex qMutex;
        // Mutex for accessing source
        std::shared_mutex sMutex;
        // Source currently playing
        Source * source;

    public:
        // Constructor initializes socket related things
        MainService();

        // Call to exit and prepare for deletion (will stop loop)
        void exit();

        // Handles decoding and shifting between songs due to commands
        void audioThread();
        // listens and takes action when receiving a command
        void socketThread();

        ~MainService();
};

#endif