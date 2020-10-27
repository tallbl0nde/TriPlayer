#ifndef SCREEN_UPDATE_HPP
#define SCREEN_UPDATE_HPP

#include <mutex>
#include "ui/screen/Screen.hpp"

// Forawrd declare pointes
namespace Main {
    class Application;
};
class Updater;

namespace Screen {
    // The Update screen contains all UI relevant to checking for updates, showing the
    // changelog as well as initiating the overall update process.
    class Update : public Screen {
        public:
            // Current operation the thread object is performing
            enum class ThreadOperation {
                None,                           // Nothing (used to signal done)
                CheckUpdate,                    // Checking for an update
                Download,                       // Downloading update file
                Extract                         // Extracting update to SD
            };

        private:
            Aether::Text * heading;             // Heading which is always visible
            Aether::Image * icon;               // TriPlayer icon
            Aether::Image * bg;                 // Background image with texture
            Aether::Container * container;      // Element populated with text/buttons based on outcome

            Aether::RoundProgressBar * pbar;    // Progress bar within overlay
            Aether::Text * ptext;               // Download "statistics" in overlay

            std::atomic<double> nextProgress;   // Next progress value to set pbar to
            std::string statString;             // String to set ptext to in main thread
            std::mutex mutex;                   // Mutex protecting access to above string
            std::future<void> thread;           // Thread running communication operations
            std::atomic<bool> threadDone;       // Indicates whether thread has been processed
            ThreadOperation operation;          // The operation the thread is performing

            Aether::MessageBox * msgbox;        // Message box overlay

            Updater * updater;                  // Object used to interact with GitHub to download update

            // Methods to create and show above message box
            void createNewMsgbox();
            void presentDownloading();
            void presentInfo(const std::string &, const std::string &, const std::function<void()> &);
            void presentInfo(const std::string &);

            // Methods to update container element based on outcome of update check
            void showChangelog();
            void showError();
            void showUpToDate();

            // Callback invoked by CURL when downloading file
            void progressCallback(long long int, long long int);

        public:
            // Passed main application object
            Update(Main::Application *);

            // Update handles hiding overlay + updating status
            void update(uint32_t);

            // Set up elements and show message box
            void onLoad();

            // Delete elements created in onLoad()
            void onUnload();
    };
};


#endif