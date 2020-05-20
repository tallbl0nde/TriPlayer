#ifndef SCREEN_SPLASH_HPP
#define SCREEN_SPLASH_HPP

#include "Aether.hpp"
#include <atomic>
#include <future>

namespace Main {
    class Application;
};

namespace Screen {
    // Splash/loading screen
    class Splash : public Aether::Screen {
        private:
            Main::Application * app;

            // Elements
            Aether::Image * bg;
            Aether::Text * status;
            Aether::Text * statusNum;
            Aether::RoundProgressBar * pbar;
            Aether::Text * percent;
            Aether::Animation * anim;
            Aether::Text * hint;

            // Stages of loading process (used to update UI)
            enum LoadingStage {
                Search,     // Search music folder for paths
                Parse,      // Parse each file's metadata
                Update,     // Update/clean database
                Done
            };

            // Function which runs alongside UI to load stuff
            void processFiles();

            // === The following variables are used to communicate between threads === //
            // Future for thread
            std::future<void> future;
            // Stage of 'loading'
            std::atomic<LoadingStage> currentStage;
            // Value of last lStage
            LoadingStage lastStage;

            // == Stage 1: Reading file metadata ==
            // Variable which signals how many files have been read (set to 0 until files have been found)
            std::atomic<int> currentFile;
            // Total number of files found
            std::atomic<int> totalFiles;
            // Used to detect change
            int lastFile;

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