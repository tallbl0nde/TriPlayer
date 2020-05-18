#include <algorithm>
#include <ctime>
#include "PlayQueue.hpp"
#include <random>

// Maximum number of IDs (reserved at creation)
#define MAX_SIZE 30000  // Requires 180kB (120kB for IDs, 60kB for position)

PlayQueue::PlayQueue() {
    this->idx = 0;
    this->queue.reserve(MAX_SIZE);
    this->shuffled = false;
}

bool PlayQueue::addID(SongID id, unsigned short pos) {
    // Sanity check
    if (this->queue.size() == MAX_SIZE) {
        return false;
    }

    // If past the end add at end
    if (pos > this->queue.size()) {
        pos = this->queue.size();
    }

    PlayQueuePair p;
    p.id = id;
    p.pos = (this->shuffled ? this->maxPos + 1 : pos);
    this->queue.insert(this->queue.begin() + pos, p);

    if (!this->shuffled) {
        // If it's not shuffled we update following pairs
        for (size_t i = pos + 1; i < this->queue.size(); i++) {
            this->queue[i].pos++;
        }
    }
    this->maxPos++;

    return true;
}

bool PlayQueue::removeID(unsigned short pos) {
    // Sanity check
    if (this->queue.size() == MAX_SIZE || pos >= this->queue.size()) {
        return false;
    }

    this->queue.erase(this->queue.begin() + pos);
    if (!this->shuffled) {
        // If it's not shuffled we update following pairs
        for (size_t i = pos + 1; i < this->queue.size(); i++) {
            this->queue[i].pos--;
        }
        this->maxPos--;
    }

    return true;
}

void PlayQueue::moveIDDown(unsigned short pos, unsigned short amt) {
    // Sanity check
    if (pos >= this->queue.size() || amt == 0) {
        return;
    }

    // If too large move to the end
    if ((pos + amt) > this->queue.size()) {
        amt = (this->queue.size() - pos);
    }

    // Move (need to update positions if not shuffled)
    if (!this->shuffled) {
        for (size_t i = pos + 1; i < pos + amt; i++) {
            PlayQueuePair tmp = this->queue[i-1];
            this->queue[i-1] = this->queue[i];
            this->queue[i] = tmp;
        }

    } else {
        for (size_t i = pos + 1; i < pos + amt; i++) {
            PlayQueuePair tmp = this->queue[i-1];
            this->queue[i-1] = this->queue[i];
            this->queue[i] = tmp;
            this->queue[i-1].pos--;
            this->queue[i].pos++;
        }
    }
}

void PlayQueue::moveIDUp(unsigned short pos, unsigned short amt) {
    // Sanity check
    if (pos == 0 || pos >= this->queue.size() || amt == 0) {
        return;
    }

    // If too large move to the start
    if (amt > pos) {
        amt = pos;
    }

    // Move (need to update positions if not shuffled)
    unsigned short mv = pos - amt;
    if (!this->shuffled) {
        for (size_t i = pos - 1; i >= mv; i--) {
            PlayQueuePair tmp = this->queue[i+1];
            this->queue[i+1] = this->queue[i];
            this->queue[i] = tmp;
        }

    } else {
        for (size_t i = pos - 1; i >= mv; i--) {
            PlayQueuePair tmp = this->queue[i+1];
            this->queue[i+1] = this->queue[i];
            this->queue[i] = tmp;
            this->queue[i+1].pos++;
            this->queue[i].pos--;
        }
    }
}

SongID PlayQueue::currentID() {
    if (this->queue.size() == 0) {
        return -1;
    }

    return this->queue[this->idx].id;
}

SongID PlayQueue::IDatPosition(unsigned short pos) {
    if (this->queue.size() == 0 || pos >= this->queue.size()) {
        return -1;
    }

    return this->queue[pos].id;
}

size_t PlayQueue::currentIdx() {
    return this->idx;
}

void PlayQueue::decrementIdx() {
    if (this->idx == 0) {
        return;
    }

    this->idx--;
}

void PlayQueue::incrementIdx() {
    if (this->idx == this->queue.size() - 1) {
        return;
    }

    this->idx++;
}

void PlayQueue::setIdx(unsigned short i) {
    if (i >= this->queue.size()) {
        this->idx = this->queue.size() - 1;
    } else {
        this->idx = i;
    }
}

void PlayQueue::clear() {
    this->idx = 0;
    this->queue.erase(this->queue.begin(), this->queue.end());
    this->shuffled = false;
}

size_t PlayQueue::size() {
    return this->queue.size();
}

bool PlayQueue::isShuffled() {
    return this->shuffled;
}

void PlayQueue::shuffle() {
    if (this->queue.size() == 0) {
        return;
    }

    // RNG
    std::default_random_engine gen;
    gen.seed(std::time(nullptr));

    // Set current song as first
    PlayQueuePair tmp = this->queue[this->idx];
    this->queue[this->idx] = this->queue[0];
    this->queue[0] = tmp;
    this->setIdx(0);

    // Uses the Yates-Fisher algorithm
    for (size_t i = this->queue.size() - 1; i > 1; i--) {
        std::uniform_int_distribution<int> dist(1, i);
        int r = dist(gen);
        tmp = this->queue[i];
        this->queue[i] = this->queue[r];
        this->queue[r] = tmp;
    }

    this->shuffled = true;
}

void PlayQueue::unshuffle() {
    if (!this->shuffled) {
        return;
    }

    // Get pos of current song
    unsigned short songPos = this->queue[this->currentIdx()].pos;

    // Sort by pos value
    std::sort(this->queue.begin(), this->queue.end(), [](const PlayQueuePair & lhs, const PlayQueuePair & rhs) {
        return lhs.pos < rhs.pos;
    });

    // Update pos values
    for (size_t i = 0; i < this->queue.size(); i++) {
        // Set same song as current song
        if (this->queue[i].pos == songPos) {
            this->setIdx(i);
        }
        this->queue[i].pos = i;
    }
    this->maxPos = this->queue.size();

    this->shuffled = false;
}