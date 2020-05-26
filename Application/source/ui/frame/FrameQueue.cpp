#include "Application.hpp"
#include "dtl.hpp"
#include "FrameQueue.hpp"
#include "ListSong.hpp"
#include <sstream>
#include "Utils.hpp"

// Helper function returning length of songs in queue in seconds
unsigned int durationOfQueue(std::vector<SongID> & queue, std::vector<SongInfo> & songInfo) {
    unsigned int total = 0;

    // Get info for each song and sum up
    for (size_t i = 0; i < queue.size(); i++) {
        // I can't think of a case where the songInfo would be in there twice
        std::vector<SongInfo>::iterator it = std::lower_bound(songInfo.begin(), songInfo.end(), queue[i], [](const SongInfo info, const SongID id) {
            return info.ID < id;
        });
        if (it == songInfo.end()) {
            // If not found don't add
            continue;
        }

        total += (*it).duration;
    }

    return total;
}

namespace Frame {
    Queue::Queue(Main::Application * a) : Frame(a) {
        // Get sorted list of SongInfo (faster than iterating per song)
        this->songInfo = this->app->database()->getAllSongInfo();
        std::sort(this->songInfo.begin(), this->songInfo.end(), [](const SongInfo lhs, const SongInfo rhs) {
            return lhs.ID < rhs.ID;
        });

        this->cachedSongID = -1;
        this->emptyMsg = nullptr;
        this->heading->setString("Play Queue");
        this->playingElm = nullptr;
        this->queue = nullptr;
        this->createList();
        this->updateList();
        this->setFocussed(this->list);
        this->songPressed = false;
    }

    void Queue::initEmpty() {
        this->list->setHidden(true);
        this->subLength->setHidden(true);
        this->subTotal->setHidden(true);
        this->removeElement(this->emptyMsg);
        this->emptyMsg = new Aether::Text(0, this->list->y() + this->list->h()*0.4, "Your Play Queue is empty!", 24);
        this->emptyMsg->setColour(this->app->theme()->FG());
        this->emptyMsg->setX(this->x() + (this->w() - this->emptyMsg->w())/2);
        this->addElement(this->emptyMsg);
    }

    void Queue::createList() {
        this->list->setHidden(false);
        this->subLength->setHidden(false);
        this->subTotal->setHidden(false);

        // Now Playing
        this->playing = new Aether::Element(0, 0, 100, 45);
        Aether::Text * tmp = new Aether::Text(this->playing->x(), this->playing->y(), "Now Playing", 28);
        tmp->setColour(this->app->theme()->FG());
        this->playing->addElement(tmp);
        this->list->addElement(this->playing);
        this->playing->setY(this->list->y() + 10);

        // Queue - added in updateList() if needed!

        // Up Next
        this->upnext = new Aether::Element(0, 0, 100, 80);
        tmp = new Aether::Text(this->upnext->x(), this->upnext->y(), "Up Next", 28);
        tmp->setY(tmp->y() + (this->upnext->h() - tmp->h())/2 + 10);
        tmp->setColour(this->app->theme()->FG());
        this->upnext->addElement(tmp);
        this->list->addElement(this->upnext);
    }

    void Queue::updateList() {
        // Get queues
        std::vector<SongID> queue = this->app->sysmodule()->queue();
        std::vector<SongID> subQueue = this->app->sysmodule()->subQueue();
        size_t songIdx = this->app->sysmodule()->waitSongIdx();
        SongID currentID = -1;

        // Safety check before 'slicing' queue (if there's an error leave it empty)
        if (songIdx >= queue.size()) {
            queue.clear();
        } else {
            currentID = queue[songIdx];
            queue = std::vector<SongID>(queue.begin() + songIdx + 1, queue.end()); // Note we don't include the current song in the queue!
        }

        // Set empty if so
        if (queue.empty() && subQueue.empty()) {
            this->initEmpty();
            return;
        }

        // Set current song
        if (currentID != this->cachedSongID) {
            this->list->removeElement(this->playingElm);
            this->playingElm = this->getListSong(currentID);
            this->playingElm->setTextColour(this->app->theme()->accent());
            this->playingElm->setMoreCallback([this, currentID]() {
                // Show normal song menu (can't remove from queue!)
                this->app->showSongMenu(currentID);
            });
            this->list->addElementAfter(this->playingElm, this->playing);
        }

        // Diff each type of queue
        dtl::Diff<SongID> queueDiff(cachedQueue, queue);
        queueDiff.compose();
        dtl::Diff<SongID> subQueueDiff(cachedSubQueue, subQueue);
        subQueueDiff.compose();

        // Remove/add queue heading if needed
        if (subQueue.empty() && this->queue != nullptr) {
            this->list->removeElement(this->queue);
            this->queue = nullptr;
        } else if (!subQueue.empty() && this->queue == nullptr) {
            this->queue = new Aether::Element(0, 0, 100, 80);
            Aether::Text * tmp = new Aether::Text(this->queue->x(), this->queue->y(), "Next in Queue", 28);
            tmp->setY(tmp->y() + (this->queue->h() - tmp->h())/2 + 10);
            tmp->setColour(this->app->theme()->FG());
            this->queue->addElement(tmp);
            this->list->addElementBefore(this->queue, this->upnext);
        }

        // Update sub queue
        std::list<CustomElm::ListSong *>::iterator it = this->queueEls.begin();
        std::stringstream ss;
        subQueueDiff.printSES(ss);
        for (std::string line; std::getline(ss, line);) {
            // Take action based on first char
            switch (line[0]) {
                // Remove element
                case '-': {
                    std::list<CustomElm::ListSong *>::iterator prev = std::prev(it);
                    this->list->removeElement(*it);
                    this->queueEls.erase(it);
                    it = prev;
                    std::advance(it, 1);
                    break;
                }

                // Add element
                case '+': {
                    SongID id = std::stoi(line.substr(1, line.length() - 1));
                    size_t pos = std::distance(this->queueEls.begin(), it) + 1;
                    CustomElm::ListSong * l = this->getListSong(id);
                    l->setMoreCallback([this, id, pos]() {
                        this->app->showSongMenu(id, pos, SongMenuType::RemoveFromSubQueue);
                    });
                    if (it == this->queueEls.begin()) {
                        this->list->addElementAfter(l, this->queue);
                    } else {
                        this->list->addElementAfter(l, *std::prev(it));
                    }
                    this->queueEls.insert(it, l);
                    break;
                }

                // Don't alter if the same
                case ' ':
                    std::advance(it, 1);
                    break;
            }
        }

        // Hide 'Up Next' heading if it's empty
        if (queue.empty()) {
            this->upnext->setHidden(true);
        } else {
            this->upnext->setHidden(false);
        }

        // Update normal queue (including current song)
        it = this->upnextEls.begin();
        std::stringstream ss2;
        queueDiff.printSES(ss2);
        for (std::string line; std::getline(ss2, line);) {
            // Take action based on first char
            switch (line[0]) {
                // Remove element
                case '-': {
                    std::list<CustomElm::ListSong *>::iterator prev = std::prev(it);
                    this->list->removeElement(*it);
                    this->upnextEls.erase(it);
                    it = prev;
                    std::advance(it, 1);
                    break;
                }

                // Add element
                case '+': {
                    SongID id = std::stoi(line.substr(1, line.length() - 1));
                    size_t pos = std::distance(this->upnextEls.begin(), it) + 1;
                    CustomElm::ListSong * l = this->getListSong(id);
                    l->setMoreCallback([this, id, pos]() {
                        this->app->showSongMenu(id, pos, SongMenuType::RemoveFromQueue);
                    });
                    if (it == this->upnextEls.begin()) {
                        this->list->addElementAfter(l, this->upnext);
                    } else {
                        this->list->addElementAfter(l, *std::prev(it));
                    }
                    this->upnextEls.insert(it, l);
                    break;
                }

                // Don't alter if the same
                case ' ':
                    std::advance(it, 1);
                    break;
            }
        }

        // Update cached variables
        this->cachedSongID = currentID;
        this->cachedQueue = queue;
        this->cachedSubQueue = subQueue;

        // Update length + track strings
        std::vector<SongID> tmp = {this->cachedSongID};
        unsigned int totalSecs = durationOfQueue(this->cachedQueue, this->songInfo) + durationOfQueue(this->cachedSubQueue, this->songInfo) + durationOfQueue(tmp, this->songInfo);
        this->subLength->setString(Utils::secondsToHoursMins(totalSecs));
        this->subLength->setX(this->x() + 885 - this->subLength->w());
        unsigned int totalTracks = this->cachedQueue.size() + this->cachedSubQueue.size() + 1;  // Plus 1 for playing song
        this->subTotal->setString(std::to_string(totalTracks) + (totalTracks == 1 ? " track" : " tracks" ) + " remaining");
        this->subTotal->setX(this->x() + 885 - this->subTotal->w());
    }

    CustomElm::ListSong * Queue::getListSong(size_t id) {
        // Get info for song (will be blank if not found)
        SongInfo si;
        // I can't think of a case where the songInfo would be in there twice
        std::vector<SongInfo>::iterator it = std::lower_bound(this->songInfo.begin(), this->songInfo.end(), id, [](const SongInfo info, const SongID id) {
            return info.ID < id;
        });
        if (it != this->songInfo.end()) {
            si = *(it);
        }

        // Create element
        CustomElm::ListSong * l = new CustomElm::ListSong();
        l->setTitleString(si.title);
        l->setArtistString(si.artist);
        l->setAlbumString(si.album);
        l->setLengthString(Utils::secondsToHMS(si.duration));
        l->setDotsColour(this->app->theme()->muted());
        l->setLineColour(this->app->theme()->muted2());
        l->setTextColour(this->app->theme()->FG());
        l->setCallback([this, id](){
            this->app->sysmodule()->sendSetSongIdx(id);
            this->songPressed = true;
        });

        return l;
    }

    void Queue::update(uint32_t dt) {
        // Edit list when either queue is changed (call both to ensure only done once)
        bool b = (this->app->sysmodule()->queueChanged() || this->app->sysmodule()->subQueueChanged());
        if (b) {
            this->updateList();
        }

        Frame::update(dt);
    }
};