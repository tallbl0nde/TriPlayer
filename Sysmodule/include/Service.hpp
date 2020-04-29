#ifndef MAINSERVICE_HPP
#define MAINSERVICE_HPP

#include <atomic>
#include "Audio.hpp"
#include "Database.hpp"
#include "MP3.hpp"
#include <mutex>

// Class which manages all actions taken when receiving a command
// Essentially encapsulates everything
class MainService {
    private:
        // Audio instance
        Audio * audio;

        // Database object
        Database * db;

        // Whether to stop loop and exit
        std::atomic<bool> exit_;

        // TEMP (will be removed when queue implemented)
        SongID currentID;
        MP3 * source;
        std::mutex sMutex;  // For accessing above source
        std::atomic<bool> skip; // Stop waiting for buffer and change song

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