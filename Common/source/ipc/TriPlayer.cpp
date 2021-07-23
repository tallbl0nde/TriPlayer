#include "ipc/Command.hpp"
#include "ipc/TriPlayer.hpp"
#include <string.h>
#include <switch.h>

namespace TriPlayer {
    static Service * service = nullptr;         // Service object used for communication

    bool initialize() {
        // Return true if already initialized
        if (service != nullptr) {
            return true;
        }

        // Check if service exists
        SmServiceName name = smEncodeName("tri");
        uint8_t exists;
        Result rc = tipcDispatchInOut(smGetServiceSessionTipc(), 65100, name, exists);
        if (!(R_SUCCEEDED(rc) && exists)) {
            return false;
        }

        // Acquire service object
        service = new Service;
        rc = smGetServiceWrapper(service, name);
        if (R_FAILED(rc)) {
            delete service;
            service = nullptr;
        }
        return (service != nullptr);
    }

    void exit() {
        if (service != nullptr) {
            serviceClose(service);
            delete service;
            service = nullptr;
        }
    }

    bool getVersion(std::string & outVersion) {
        // Permits xx.xx.xx (i.e. 2 digits each)
        char version[10] = {0};

        Result rc = serviceDispatchOut(service, static_cast<uint32_t>(Ipc::Command::Version), version);
        if (R_FAILED(rc)) {
            return false;
        }

        outVersion = std::string(version);
        return true;
    }

    bool resume() {
        return (R_SUCCEEDED(serviceDispatch(service, static_cast<uint32_t>(Ipc::Command::Resume))));
    }

    bool pause() {
        return (R_SUCCEEDED(serviceDispatch(service, static_cast<uint32_t>(Ipc::Command::Pause))));
    }

    bool previous() {
        return (R_SUCCEEDED(serviceDispatch(service, static_cast<uint32_t>(Ipc::Command::Previous))));
    }

    bool next() {
        return (R_SUCCEEDED(serviceDispatch(service, static_cast<uint32_t>(Ipc::Command::Next))));
    }

    bool getVolume(double & outVolume) {
        return (R_SUCCEEDED(serviceDispatchOut(service, static_cast<uint32_t>(Ipc::Command::GetVolume), outVolume)));
    }

    bool setVolume(const double volume) {
        return (R_SUCCEEDED(serviceDispatchIn(service, static_cast<uint32_t>(Ipc::Command::SetVolume), volume)));
    }

    bool mute() {
        return (R_SUCCEEDED(serviceDispatch(service, static_cast<uint32_t>(Ipc::Command::Mute))));
    }

    bool unmute(double & outVolume) {
        return (R_SUCCEEDED(serviceDispatchOut(service, static_cast<uint32_t>(Ipc::Command::Unmute), outVolume)));
    }

    bool getSubQueue(std::vector<int> & outIDs) {
        // Request queue in groups of 100
        constexpr size_t count = 100;
        outIDs.clear();

        // Repeatedly request groups until we run out
        size_t offset = 0;
        while (true) {
            // Prepare to handle received data
            const struct {
               size_t index;
               size_t count;
            } in = {offset, count};
            outIDs.resize(offset + count);

            // Request data
            size_t returned = 0;
            Result rc = serviceDispatchInOut(service, static_cast<uint32_t>(Ipc::Command::GetSubQueue), in, returned,
                .buffer_attrs = {SfBufferAttr_Out | SfBufferAttr_HipcMapAlias},
                .buffers = {{&outIDs[offset], count * sizeof(int)}},
            );
            offset += returned;
            if (R_FAILED(rc)) {
                return false;
            }

            // Stop if we didn't receive the amount requested (means we've got the entire sub queue)
            if (returned != count) {
                outIDs.resize(offset);
                break;
            }
        }

        return true;
    }

    bool getSubQueueSize(size_t & outCount) {
        return (R_SUCCEEDED(serviceDispatchOut(service, static_cast<uint32_t>(Ipc::Command::SubQueueSize), outCount)));
    }

    bool addToSubQueue(const int ID) {
        return (R_SUCCEEDED(serviceDispatchIn(service, static_cast<uint32_t>(Ipc::Command::AddToSubQueue), ID)));
    }

    bool removeFromSubQueue(const size_t pos) {
        return (R_SUCCEEDED(serviceDispatchIn(service, static_cast<uint32_t>(Ipc::Command::RemoveFromSubQueue), pos)));
    }

    bool skipSubQueueSongs(const size_t count) {
        size_t skipped;
        Result rc = serviceDispatchInOut(service, static_cast<uint32_t>(Ipc::Command::SkipSubQueueSongs), count, skipped);
        return (R_SUCCEEDED(rc));
    }

    bool getQueue(std::vector<int> & outIDs) {
        // Request queue in groups of 100
        constexpr size_t count = 100;
        outIDs.clear();

        // Repeatedly request groups until we run out
        size_t offset = 0;
        while (true) {
            // Prepare to handle received data
            const struct {
               size_t index;
               size_t count;
            } in = {offset, count};
            outIDs.resize(offset + count);

            // Request data
            size_t returned = 0;
            Result rc = serviceDispatchInOut(service, static_cast<uint32_t>(Ipc::Command::GetQueue), in, returned,
                .buffer_attrs = {SfBufferAttr_Out | SfBufferAttr_HipcMapAlias},
                .buffers = {{&outIDs[offset], count * sizeof(int)}},
            );
            offset += returned;
            if (R_FAILED(rc)) {
                return false;
            }

            // Stop if we didn't receive the amount requested (means we've got the entire sub queue)
            if (returned != count) {
                outIDs.resize(offset);
                break;
            }
        }

        return true;
    }

    bool getQueueSize(size_t & outCount) {
        return (R_SUCCEEDED(serviceDispatchOut(service, static_cast<uint32_t>(Ipc::Command::QueueSize), outCount)));
    }

    bool setQueue(const std::vector<int> & IDs) {
        size_t count;
        Result rc = serviceDispatchOut(service, static_cast<uint32_t>(Ipc::Command::SetQueue), count,
            .buffer_attrs = {SfBufferAttr_In | SfBufferAttr_HipcMapAlias},
            .buffers = {{&IDs[0], IDs.size() * sizeof(int)}},
        );
        return (R_SUCCEEDED(rc));
    }

    bool getQueueIdx(size_t & outPos) {
        return (R_SUCCEEDED(serviceDispatchOut(service, static_cast<uint32_t>(Ipc::Command::QueueIdx), outPos)));
    }

    bool setQueueIdx(const size_t pos) {
        size_t newIdx = 0;
        Result rc = serviceDispatchInOut(service, static_cast<uint32_t>(Ipc::Command::SetQueueIdx), pos, newIdx);
        if (R_FAILED(rc) || newIdx != pos) {
            return false;
        }

        return true;
    }

    bool removeFromQueue(const size_t pos) {
        return (R_SUCCEEDED(serviceDispatchIn(service, static_cast<uint32_t>(Ipc::Command::RemoveFromQueue), pos)));
    }

    bool getRepeatMode(Repeat & outMode) {
        return (R_SUCCEEDED(serviceDispatchOut(service, static_cast<uint32_t>(Ipc::Command::GetRepeat), outMode)));
    }

    bool setRepeatMode(const Repeat mode) {
        return (R_SUCCEEDED(serviceDispatchIn(service, static_cast<uint32_t>(Ipc::Command::SetRepeat), mode)));
    }

    bool getShuffleMode(Shuffle & outMode) {
        return (R_SUCCEEDED(serviceDispatchOut(service, static_cast<uint32_t>(Ipc::Command::GetShuffle), outMode)));
    }

    bool setShuffleMode(const Shuffle mode) {
        return (R_SUCCEEDED(serviceDispatchIn(service, static_cast<uint32_t>(Ipc::Command::SetShuffle), mode)));
    }

    bool getSongID(int & outID) {
        return (R_SUCCEEDED(serviceDispatchOut(service, static_cast<uint32_t>(Ipc::Command::GetSong), outID)));
    }

    bool getStatus(Status & outStatus) {
        return (R_SUCCEEDED(serviceDispatchOut(service, static_cast<uint32_t>(Ipc::Command::GetStatus), outStatus)));
    }

    bool getPosition(double & outPos) {
        return (R_SUCCEEDED(serviceDispatchOut(service, static_cast<uint32_t>(Ipc::Command::GetPosition), outPos)));
    }

    bool setPosition(const double pos) {
        double newPos;
        return (R_SUCCEEDED(serviceDispatchInOut(service, static_cast<uint32_t>(Ipc::Command::SetPosition), pos, newPos)));
    }

    bool getPlayingFromText(std::string & outText) {
        char text[101] = {0};
        Result rc = serviceDispatch(service, static_cast<uint32_t>(Ipc::Command::GetPlayingFrom),
            .buffer_attrs = {SfBufferAttr_Out | SfBufferAttr_HipcMapAlias},
            .buffers = {{text, sizeof(text)}},
        );
        if (R_FAILED(rc)) {
            return false;
        }

        outText = std::string(text);
        return true;
    }

    bool setPlayingFromText(const std::string & text) {
        // Copy string into format suitable for transmitting
        char * str = strndup(text.c_str(), 100);
        size_t len = strlen(str);

        Result rc = serviceDispatch(service, static_cast<uint32_t>(Ipc::Command::SetPlayingFrom),
            .buffer_attrs = {SfBufferAttr_In | SfBufferAttr_HipcMapAlias},
            .buffers = {{str, len + 1}},
        );
        free(str);

        if (R_FAILED(rc)) {
            return false;
        }

        return true;
    }

    bool requestDatabaseLock() {
        return (R_SUCCEEDED(serviceDispatch(service, static_cast<uint32_t>(Ipc::Command::RequestDBLock))));
    }

    bool releaseDatabaseLock() {
        return (R_SUCCEEDED(serviceDispatch(service, static_cast<uint32_t>(Ipc::Command::ReleaseDBLock))));
    }

    bool reloadConfig() {
        return (R_SUCCEEDED(serviceDispatch(service, static_cast<uint32_t>(Ipc::Command::ReloadConfig))));
    }

    bool reset() {
        return (R_SUCCEEDED(serviceDispatch(service, static_cast<uint32_t>(Ipc::Command::Reset))));
    }

    bool stopSysmodule() {
        return (R_SUCCEEDED(serviceDispatch(service, static_cast<uint32_t>(Ipc::Command::Quit))));
    }
};
