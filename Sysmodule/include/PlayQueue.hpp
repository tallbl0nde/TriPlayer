#ifndef PLAYQUEUE_HPP
#define PLAYQUEUE_HPP

#include "Types.hpp"
#include <vector>

// Struct stored in vector
typedef struct {
    SongID id;              // 4 bytes (it's an int)
    unsigned short pos;     // 2 bytes (we don't need more than 65535)
} PlayQueuePair;

// A play queue stores a list of song IDs and can be shuffled, unshuffled (due to keeping original positions)
// and have IDs inserted/removed. Even though it uses a vector there is a hard limit to avoid running out of RAM
class PlayQueue {
    private:
        // Index of 'current' song
        unsigned short idx;
        // Largest pos in queue (used for adding IDs when shuffled)
        unsigned short maxPos;
        // Vector containing IDs
        std::vector<PlayQueuePair> queue;
        // Are we shuffled?
        bool shuffled;

    public:
        PlayQueue();

        // Add an ID at given position, shifting down (returns false if full)
        bool addID(SongID, unsigned short);
        // Remove ID at given position (returns false if out of bounds)
        bool removeID(unsigned short);

        // Shift ID at position by given spots towards end (will move to end if too far)
        void moveIDDown(unsigned short, unsigned short);
        // Shift ID at position by given spots towards start (will move to start if too far)
        void moveIDUp(unsigned short, unsigned short);

        // Get the current ID (-1 if empty)
        SongID currentID();
        // Returns ID at position (-1 if out of bounds)
        SongID IDatPosition(unsigned short);

        // Get the index of the current ID
        size_t currentIdx();
        // Decrease position (does nothing if at the start)
        void decrementIdx();
        // Increase position (does nothing if at the end)
        void incrementIdx();
        // Set position (set to end if larger than size)
        void setIdx(unsigned short);

        // Clear the queue
        void clear();
        // Return number of IDs in queue
        size_t size();

        // Returns true if shuffled
        bool isShuffled();
        // (Re)shuffle the queue (current song will become the first song in queue)
        void shuffle();
        // Unshuffle the queue (no effect if not shuffled)
        void unshuffle();
};

#endif