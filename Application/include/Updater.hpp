#ifndef UPDATER_HPP
#define UPDATER_HPP

#include <string>

class Updater {
    public:
        // Metadata about an available update
        struct Meta {
            std::string changelog;
            size_t size;
            std::string version;
        };

    private:
        Meta meta;                  // Above struct which is filled
        std::string downloadUrl;    // Url to download update from

    public:
        // Instantiate a new updater
        Updater();

        // Returns whether an update is available based on the local file
        bool availableUpdate();

        // Check for an update, returning if one is found
        // This will block until the process completes
        bool checkForUpdate();

        // Return the metadata for an update (must be called after checkForUpdate())
        Meta getMetadata();

        // Returns whether enough time has passed since the last check
        // Passed value is the 'threshold'
        bool needsCheck(const size_t);

        // Download the update (if one is available)
        // This blocks until done and returns whether successful
        bool downloadUpdate(std::function<void(long long, long long)>);
};

#endif