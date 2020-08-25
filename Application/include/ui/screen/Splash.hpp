#ifndef SCREEN_SPLASH_HPP
#define SCREEN_SPLASH_HPP

#include "Aether/Aether.hpp"
#include <future>

namespace Main {
    class Application;
};

namespace Screen {
    // The 'Splash' screen is shown when the application is launched.
    // It is displayed while the library is scanned and the database
    // is updated. It portrays the progress of the update.
    class Splash : public Aether::Screen {
        private:
            // Stages in the library scan
            enum class ScanStage {
                Launch,         // Not scanning yet
                Files,          // Searching for file changes
                Metadata,       // Extracting metadata from files
                Database,       // Updating database to match filesystem
                Art,            // Extracting album art from needd files
                Done,           // Everything is done
                Error           // An error occurred during the scan
            };

            // Pointer to main app object
            Main::Application * app;

            // Set true when an error has occurred (allows exit)
            bool fatalError;

            // UI elements
            Aether::Image * bg;
            Aether::Text * heading;
            Aether::Text * subheading;
            Aether::RoundProgressBar * progress;
            Aether::Text * percent;
            Aether::Animation * animation;
            Aether::Text * hint;
            Aether::BorderButton * launch;
            Aether::BorderButton * quit;

            // Helper functions to update UI
            void setErrorConnect();
            void setErrorVersion();
            void setScanLaunch();
            void setScanFiles();
            void setScanMetadata();
            void setScanDatabase();
            void setScanArt();
            void setScanError();

            // === Variables to communicate status between threads === //
            // Future for thread
            std::future<void> future;
            // Stage in scan
            std::atomic<ScanStage> currentStage;
            ScanStage lastStage;
            // Number of scanned (completed) files
            std::atomic<size_t> currentFile;
            // Total number of files to scan/update
            std::atomic<size_t> totalFiles;
            // Used to detect change
            size_t lastFile;
            // Number of estimated seconds remaining
            std::atomic<size_t> estRemaining;

            // Function run on another thread to control library scan
            void scanLibrary();

        public:
            Splash(Main::Application *);

            // Update contains logic to update UI based on loading status
            void update(uint32_t);

            // onLoad creates all elements
            void onLoad();

            // onUnload deletes all elements
            void onUnload();
    };
};

#endif