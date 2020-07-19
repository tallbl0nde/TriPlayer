#include <sstream>
#include "Application.hpp"
#include "dtl.hpp"
#include "ui/element/ListSong.hpp"
#include "ui/frame/Queue.hpp"
#include "utils/MP3.hpp"
#include "utils/Utils.hpp"

// Helper function returning length of songs in queue in seconds
unsigned int durationOfQueue(std::vector<SongID> & queue, std::vector<Metadata::Song> & songMeta) {
    unsigned int total = 0;

    // Get info for each song and sum up
    for (size_t i = 0; i < queue.size(); i++) {
        // I can't think of a case where the same metadata would be in there twice
        std::vector<Metadata::Song>::iterator it = std::lower_bound(songMeta.begin(), songMeta.end(), queue[i], [](const Metadata::Song info, const SongID id) {
            return info.ID < id;
        });
        if (it == songMeta.end()) {
            // If not found don't add
            continue;
        }

        total += (*it).duration;
    }

    return total;
}

namespace Frame {
    Queue::Queue(Main::Application * a) : Frame(a) {
        // Get sorted list of metadata (faster than iterating per song)
        this->songMeta = this->app->database()->getAllSongMetadata();
        std::sort(this->songMeta.begin(), this->songMeta.end(), [](const Metadata::Song lhs, const Metadata::Song rhs) {
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
        this->menu = nullptr;
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
        this->upnextStr = new Aether::Text(this->upnext->x(), this->upnext->y(), "Up Next", 28);
        this->upnextStr->setY(this->upnextStr->y() + (this->upnext->h() - this->upnextStr->h())/2 + 10);
        this->upnextStr->setColour(this->app->theme()->FG());
        this->upnext->addElement(this->upnextStr);
        this->list->addElement(this->upnext);
    }

    void Queue::updateList() {
        // Get queues
        std::string playingFrom = this->app->sysmodule()->playingFrom();
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
        if (currentID == -1 && queue.empty() && subQueue.empty()) {
            this->initEmpty();
            return;
        }

        // Set current song
        if (currentID != this->cachedSongID) {
            this->list->removeElement(this->playingElm);
            this->playingElm = this->getListSong(currentID, Section::Playing);
            this->playingElm->setTextColour(this->app->theme()->accent());
            this->playingElm->setMoreCallback([this, currentID]() {
                // Show normal song menu (can't remove from queue!)
                this->createMenu(currentID, 0, Section::Playing);
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
                    CustomElm::ListSong * l = this->getListSong(id, Section::Queue);
                    l->setMoreCallback([this, id, l]() {
                        // Need to find current position
                        std::list<CustomElm::ListSong *>::iterator it = std::find(this->queueEls.begin(), this->queueEls.end(), l);
                        size_t pos = std::distance(this->queueEls.begin(), it);
                        this->createMenu(id, pos, Section::Queue);
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

        // Hide 'Up Next' heading if it's empty (note Aether only rerenders if the string changes)
        this->upnextStr->setString("Up Next from: " + playingFrom);
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
                    CustomElm::ListSong * l = this->getListSong(id, Section::UpNext);
                    l->setMoreCallback([this, id, l]() {
                        // Need to find current position
                        std::list<CustomElm::ListSong *>::iterator it = std::find(this->upnextEls.begin(), this->upnextEls.end(), l);
                        size_t pos = std::distance(this->upnextEls.begin(), it) + this->cachedSongIdx + 1;
                        this->createMenu(id, pos, Section::UpNext);
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
        this->cachedSongIdx = songIdx;
        this->cachedSongID = currentID;
        this->cachedQueue = queue;
        this->cachedSubQueue = subQueue;

        // Update length + track strings
        std::vector<SongID> tmp = {this->cachedSongID};
        unsigned int totalSecs = durationOfQueue(this->cachedQueue, this->songMeta) + durationOfQueue(this->cachedSubQueue, this->songMeta) + durationOfQueue(tmp, this->songMeta);
        this->subLength->setString(Utils::secondsToHoursMins(totalSecs));
        this->subLength->setX(this->x() + 885 - this->subLength->w());
        unsigned int totalTracks = this->cachedQueue.size() + this->cachedSubQueue.size() + 1;  // Plus 1 for playing song
        this->subTotal->setString(std::to_string(totalTracks) + (totalTracks == 1 ? " track" : " tracks" ) + " remaining");
        this->subTotal->setX(this->x() + 885 - this->subTotal->w());
    }

    CustomElm::ListSong * Queue::getListSong(size_t id, Section sec) {
        // Get info for song (will be blank if not found)
        Metadata::Song m;
        // I can't think of a case where the same metadata would be in there twice
        std::vector<Metadata::Song>::iterator it = std::lower_bound(this->songMeta.begin(), this->songMeta.end(), id, [](const Metadata::Song info, const SongID id) {
            return info.ID < id;
        });
        if (it != this->songMeta.end()) {
            m = *(it);
        }

        // Create element
        CustomElm::ListSong * l = new CustomElm::ListSong();
        l->setTitleString(m.title);
        l->setArtistString(m.artist);
        l->setAlbumString(m.album);
        l->setLengthString(Utils::secondsToHMS(m.duration));
        l->setDotsColour(this->app->theme()->muted());
        l->setLineColour(this->app->theme()->muted2());
        l->setTextColour(this->app->theme()->FG());

        if (sec == Section::Playing) {
            // Do nothing if playing but allow to be selected
            l->setCallback(nullptr);

        } else if (sec == Section::Queue) {
            // Set sub queue idx
            l->setCallback([this, l](){
                // Calculate distance since first song in queue
                size_t num = std::distance(this->queueEls.begin(), std::find(this->queueEls.begin(), this->queueEls.end(), l));

                this->app->sysmodule()->sendSkipSubQueueSongs(num);
                this->songPressed = true;
            });

        } else if (sec == Section::UpNext) {
            // Set up next idx
            l->setCallback([this, l](){
                // Calculate distance since start (find will always return within list)
                size_t idx = std::distance(this->upnextEls.begin(), std::find(this->upnextEls.begin(), this->upnextEls.end(), l));
                idx += this->app->sysmodule()->songIdx();

                // Need to add one for first song
                this->app->sysmodule()->sendSetSongIdx(idx + 1);
                this->songPressed = true;
            });
        }

        return l;
    }

    void Queue::update(uint32_t dt) {
        // Edit list when either queue is changed (call both to ensure only done once)
        bool b = (this->app->sysmodule()->queueChanged() || this->app->sysmodule()->subQueueChanged() || this->cachedSongIdx != this->app->sysmodule()->songIdx());
        if (b) {
            this->updateList();
        }

        // Jump to start if list if a song is pressed
        if (this->songPressed) {
            this->songPressed = false;
            if (this->upnextEls.size() > 0) {
                this->list->setFocussed(*(this->upnextEls.begin()));
                this->list->setScrollPos(0);
            }
        }

        Frame::update(dt);
    }

    void Queue::createMenu(SongID id, size_t pos, Section sec) {
        delete this->menu;

        // Want to show the 'Remove from Queue' based on section invoked from
        switch (sec) {
            case Section::Playing:
                this->menu = new CustomOvl::Menu::Song(CustomOvl::Menu::Song::Type::HideRemove);
                break;

            case Section::Queue:
                this->menu = new CustomOvl::Menu::Song(CustomOvl::Menu::Song::Type::ShowRemove);
                this->menu->setRemoveFromQueueText("Remove from Queue");
                this->menu->setRemoveFromQueueFunc([this, pos]() {
                    this->app->sysmodule()->sendRemoveFromSubQueue(pos);
                    this->menu->close();
                });
                break;

            case Section::UpNext:
                this->menu = new CustomOvl::Menu::Song(CustomOvl::Menu::Song::Type::ShowRemove);
                this->menu->setRemoveFromQueueText("Remove from Queue");
                this->menu->setRemoveFromQueueFunc([this, pos]() {
                    this->app->sysmodule()->sendRemoveFromQueue(pos);
                    this->menu->close();
                });
                break;
        }
        this->menu->setAddToQueueText("Add to Queue");
        this->menu->setAddToPlaylistText("Add to Playlist");
        this->menu->setGoToArtistText("Go to Artist");
        this->menu->setGoToAlbumText("Go to Album");
        this->menu->setViewInformationText("View Information");
        this->menu->setBackgroundColour(this->app->theme()->popupBG());
        this->menu->setIconColour(this->app->theme()->muted());
        this->menu->setLineColour(this->app->theme()->muted2());
        this->menu->setMutedTextColour(this->app->theme()->muted());
        this->menu->setTextColour(this->app->theme()->FG());

        // Song metadata
        Metadata::Song m = this->app->database()->getSongMetadataForID(id);
        this->menu->setTitle(m.title);
        this->menu->setArtist(m.artist);

        // Album art (this will likely be improved at some point)
        bool hasArt = false;
        if (m.path.length() > 0) {
            Metadata::Art art = Utils::MP3::getArtFromID3(m.path);
            if (art.data != nullptr) {
                this->menu->setAlbum(new Aether::Image(0, 0, art.data, art.size));
                hasArt = true;
                delete[] art.data;
            }
        }
        if (!hasArt) {
            this->menu->setAlbum(new Aether::Image(0, 0, "romfs:/misc/noalbum.png"));
        }

        // Callbacks
        this->menu->setAddToQueueFunc([this, id]() {
            this->app->sysmodule()->sendAddToSubQueue(id);
            this->menu->close();
        });
        this->menu->setAddToPlaylistFunc([this, id]() {
            // add to playlist
        });
        this->menu->setGoToArtistFunc([this, id]() {
            ArtistID a = this->app->database()->getArtistIDForSong(id);
            if (a >= 0) {
                this->changeFrame(Type::Artist, Action::Push, a);
            }
            this->menu->close();
        });
        this->menu->setGoToAlbumFunc([this, id]() {
            // go to album
        });
        this->menu->setViewInformationFunc([this, id]() {
            // view information
        });

        this->menu->resetHighlight();
        this->app->addOverlay(this->menu);
    }

    Queue::~Queue() {
        delete this->menu;
    }
};