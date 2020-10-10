#ifndef MAINSERVICE_HPP
#define MAINSERVICE_HPP

#include <atomic>
#include <ctime>
#include <deque>
#include <shared_mutex>
#include "Config.hpp"
#include "Database.hpp"
#include "nx/Audio.hpp"
#include "PlayQueue.hpp"
#include "Protocol.hpp"
#include "socket/Listener.hpp"
#include "sources/Source.hpp"

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
        // Main queue of songs
        PlayQueue * queue;
        // Queue of 'queued' songs
        std::deque<SongID> subQueue;
        // Listening socket to listen and accept new connections
        Socket::Listener * listener;

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
        // Whether to listen for GPIO events
        std::atomic<bool> watchGpio;

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

        // Function run by transfer sockets
        void commandThread(Socket::Transfer *);

    public:
        // Constructor initializes socket related things
        MainService();

        // Call to exit and prepare for deletion (will stop loop)
        void exit();

        // Listens for 'headphones unplugged' event and pauses playback
        void gpioEventThread();
        // Listens for input events and executes required commands on button presses
        void hidEventThread();
        // Handles decoding and shifting between songs due to commands
        void playbackThread();
        // Listens for 'sleep' event and pauses playback
        void sleepEventThread();
        // Listens for connections and spawns new threads for each connection
        void socketThread();

        ~MainService();
};

#endif