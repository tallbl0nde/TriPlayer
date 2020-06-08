#ifndef SCREEN_SPLASH_HPP
#define SCREEN_SPLASH_HPP

#include "Aether/Aether.hpp"
#include <future>
#include "Utils.hpp"

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

            // === The following variables are used to communicate between threads === //
            // Future for thread
            std::future<void> future;
            // Stage of 'loading'
            std::atomic<ProcessStage> currentStage;
            // Value of last lStage
            ProcessStage lastStage;
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