#ifndef MAINSERVICE_HPP
#define MAINSERVICE_HPP

#include <atomic>
#include <ctime>
#include <deque>
#include <shared_mutex>
#include "ipc/Command.hpp"
#include "ipc/Result.hpp"
#include "ipc/Server.hpp"
#include "Types.hpp"

// Forward declare pointers
class Audio;
class Config;
class Database;
class PlayQueue;
class Source;

// Class which manages all actions taken when receiving a command
// Essentially encapsulates everything
class MainService {
    private:
        // Enum specifying what action to take when changing a song (i.e. getting a new source)
        enum class SongAction {
            Previous,   // Go to the last song in the queue (if there is one)
            Next,       // Skip to the next song in the queue (if there is one)
            Replay,     // Restart the currently playing song
            Nothing     // Do nothing
        };

        // Audio instance
        Audio * audio;
        // Config object
        Config * cfg;
        // Database object
        Database * db;
        // IPC Server which clients interact with
        Ipc::Server * ipcServer;
        // Main queue of songs
        PlayQueue * queue;
        // Queue of 'queued' songs
        std::deque<SongID> subQueue;

        // Whether to stop loop and exit
        std::atomic<bool> exit_;
        // Volume when muted (used to unmute)
        std::atomic<double> muteLevel;
        // String set by client indicating where the music is playing from
        std::string playingFrom;
        // Timestamp of last previous press
        std::atomic<std::time_t> pressTime;
        // Repeat mode
        std::atomic<RepeatMode> repeatMode;
        // Status vars for comm. between threads
        std::atomic<SongAction> songAction; // (should this be a queue?)
        std::atomic<double> seekTo;
        // Whether to listen for events
        std::atomic<bool> watchGpio;
        std::atomic<bool> watchHid;
        std::atomic<bool> watchSleep;

        // Mutex for accessing queue
        std::shared_mutex qMutex;
        // Mutex for accessing source
        std::shared_mutex sMutex;
        // Mutex for accessing sub-queue
        std::shared_mutex sqMutex;
        // Source currently playing
        Source * source;

        // Mutex for access combo strings
        std::shared_mutex cMutex;
        // Variables for reacting to press combinations
        std::atomic<bool> combosUpdated;
        std::string comboNextString;
        std::string comboPlayString;
        std::string comboPrevString;

        // Variables used for 'locking' DB access
        std::mutex dbMutex;
        std::atomic<bool> dbLocked;

        // Reads config from disk and sets up relevant objects
        void updateConfig();

        // Function run to handle an IPC Request
        Ipc::Result commandThread(Ipc::Request *);

    public:
        // Constructor initializes everything
        MainService();

        // Call to exit and prepare for deletion (will stop loop)
        void exit();

        // Listens for 'headphones unplugged' event and pauses playback
        void gpioEventThread();
        // Listens for input events and executes required commands on button presses
        void hidEventThread();
        // Handles interactions from client(s)
        void ipcThread();
        // Handles decoding and shifting between songs due to commands
        void playbackThread();
        // Listens for 'sleep' event and pauses playback
        void sleepEventThread();

        ~MainService();
};

#endif