#include "Application.hpp"
#include "FrameQueue.hpp"
#include <limits>
#include "ListSong.hpp"
#include "Utils.hpp"

namespace Frame {
    Queue::Queue(Main::Application * a) : Frame(a) {
        this->emptyMsg = nullptr;
        this->heading->setString("Play Queue");
        this->initList();
        this->setFocussed(this->list);
        this->songPressed = false;
    }

    void Queue::initEmpty() {
        this->list->setHidden(true);
        this->removeElement(this->emptyMsg);
        this->emptyMsg = new Aether::Text(0, this->list->y() + this->list->h()*0.4, "Your Play Queue is empty!", 24);
        this->emptyMsg->setColour(this->app->theme()->FG());
        this->emptyMsg->setX(this->x() + (this->w() - this->emptyMsg->w())/2);
        this->addElement(this->emptyMsg);
    }

    void Queue::initList() {
        Aether::ThreadPool::removeQueuedTasks();
        Aether::ThreadPool::waitUntilDone();
        this->list->removeAllElements();
        this->elements.clear();
        this->list->setHidden(false);

        // Create items for songs in queue
        this->songIDs = this->app->sysmodule()->queue();
        this->songIdx = this->app->sysmodule()->waitSongIdx();
        if (this->songIdx == std::numeric_limits<size_t>::max() || this->songIDs.empty()) {
            this->initEmpty();
            return;
        }

        // Get sorted list of SongInfo (faster than iterating per song)
        this->songInfo = this->app->database()->getAllSongInfo();
        std::sort(this->songInfo.begin(), this->songInfo.end(), [](const SongInfo lhs, const SongInfo rhs) {
            return lhs.ID < rhs.ID;
        });

        for (size_t i = this->songIdx; i < this->songIDs.size(); i++) {
            CustomElm::ListSong * l = this->getListSong(i);
            if (i == this->songIdx) {
                l->setTextColour(this->app->theme()->accent());
            }
            this->elements.push_back(l);
            this->list->addElement(l);

            if (i == this->songIdx) {
                l->setY(this->list->y() + 10);
            }
        }
        this->updateStrings();

        // Discard songinfo
        this->songInfo.clear();
    }

    void Queue::updateStrings() {
        this->songIdx = this->app->sysmodule()->waitSongIdx();
        this->subTotal->setString(std::to_string(this->songIDs.size() - this->songIdx) + (this->songIDs.size() - this->songIdx == 1 ? " track" : " tracks" ) + " remaining");
        this->subTotal->setX(1205 - this->subTotal->w());
    }

    CustomElm::ListSong * Queue::getListSong(size_t i) {
        // Get info for song
        SongInfo si;
        if (this->songInfo.size() <= 20) {
            // Don't search if it's small or non-existant
            si = this->app->database()->getSongInfoForID(this->songIDs[i]);
        } else {
            // I can't think of a case where the songInfo would be in there twice
            std::vector<SongInfo>::iterator it = std::lower_bound(this->songInfo.begin(), this->songInfo.end(), this->songIDs[i], [](const SongInfo info, const SongID id) {
                return info.ID < id;
            });
            if (it != this->songInfo.end()) {
                si = *(it);
            }
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
        l->setCallback([this, i](){
            this->app->sysmodule()->sendSetSongIdx(i);
            this->songPressed = true;
        });
        SongID id = si.ID;
        l->setMoreCallback([this, i, id]() {
            if (i == this->songIdx) {
                this->app->showSongMenu(id);
            } else {
                this->app->showSongMenu(id, i);
            }
        });

        return l;
    }

    void Queue::update(uint32_t dt) {
        // Completely reinit list if queue is majorly changed
        if (this->app->sysmodule()->queueChanged()) {
            this->initList();
        } else {
            // Update if current index changes
            size_t newIdx = this->app->sysmodule()->songIdx();
            if (newIdx < this->songIdx) {
                // Just reinit if we've wrapped around
                if (newIdx == 0 && this->songIdx == this->songIDs.size() - 1) {
                    this->initList();

                } else {
                    // Insert required IDs
                    this->elements[0]->setTextColour(this->app->theme()->FG());
                    for (size_t i = newIdx; i < this->songIdx; i++) {
                        CustomElm::ListSong * l = this->getListSong(i);
                        this->list->addElementBefore(l, this->elements[0]);
                        this->elements.insert(this->elements.begin(), l);
                    }
                    this->elements[0]->setTextColour(this->app->theme()->accent());
                }

            } else if (newIdx > this->songIdx) {
                // Just reinit if we've wrapped around
                if (newIdx == this->songIDs.size() - 1 && this->songIdx == 0) {
                    this->initList();

                } else {
                    // Remove elements before
                    size_t offset = newIdx - this->songIdx;
                    this->elements[0]->setTextColour(this->app->theme()->FG());
                    this->list->removeElementsBefore(this->elements[offset]);
                    this->elements = std::vector(this->elements.begin() + offset, this->elements.end());
                    this->elements[0]->setTextColour(this->app->theme()->accent());
                }
            }

            // Update relevant things on any change
            if (newIdx != this->songIdx) {
                // This sets this->songIdx!!
                this->updateStrings();

                // Jump to selected element when pressed
                if (this->songPressed) {
                    this->songPressed = false;
                    this->list->setScrollPos(0);
                }
            }
        }

        Frame::update(dt);
    }
};