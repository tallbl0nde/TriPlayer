#ifndef SCREEN_SPLASH_HPP
#define SCREEN_SPLASH_HPP

#include "Application.hpp"
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

            // Future for thread
            std::future<void> future;
            // Function which runs alongside UI to load stuff
            void processFiles();
            // Variable which signals how many files have been read
            // (set to 0 until files have been found)
            std::atomic<int> currentFile;
            // UI thread uses this variable to detect change
            int lastFile;
            // Total number of files found
            std::atomic<int> totalFiles;

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