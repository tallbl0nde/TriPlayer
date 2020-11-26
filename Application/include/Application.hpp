#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <array>
#include "Config.hpp"
#include "db/SyncDatabase.hpp"
#include <future>
#include <stack>
#include "Sysmodule.hpp"
#include "ui/Theme.hpp"

// Forward declaration because cyclic dependency /shrug
namespace Screen {
    class Screen;
};

namespace Main {
    // Enumeration for screens (allows for easy switching)
    enum class ScreenID {
        Fullscreen = 0,
        Home = 1,
        Settings = 2,
        Splash = 3,
        Update = 4
    };

    // The Application class represents the "root" object of the app. It stores/handles all states
    // and objects used through the app
    class Application {
        private:
            // Display object used for rendering
            Aether::Display * display;

            // Screens of the app
            ScreenID screenID;
            std::array<Screen::Screen *, 5> screens;
            std::stack<ScreenID> screenIDs;

            // Overlay to show when prompting to exit
            Aether::MessageBox * exitPrompt;
            void createExitPrompt();

            // Config object (used to interact with config files)
            Config * config_;

            // Database object (all calls are wrapped with a mutex)
            SyncDatabase database_;

            // Sysmodule object which allows communication
            Sysmodule * sysmodule_;

            // Theme object
            Theme * theme_;

            // Thread which checks for an update
            std::atomic<bool> hasUpdate_;
            std::future<void> updateThread;

            // Thread which handles sysmodule communication
            std::future<void> sysThread;

        public:
            // Constructor inits Aether, screens + other objects
            Application();

            // Wrappers for display functions
            void setHoldDelay(int);
            void setHighlightAnimation(std::function<Aether::Colour(uint32_t)>);

            // Pass an overlay element in order to render
            // Element is not deleted when closed!
            void addOverlay(Aether::Overlay *);

            // Screen manipulation functions
            void setScreen(ScreenID);
            void pushScreen();
            void popScreen();
            void dropScreen();
            void updateScreenTheme();

            // Helper functions for database
            void lockDatabase();
            void unlockDatabase();

            // Returns whether an update is available
            bool hasUpdate();
            // Set whether the application has an update
            void setHasUpdate(const bool);

            // Returns config pointer
            Config * config();
            // Returns database object
            const SyncDatabase & database();
            // Returns sysmodule pointer
            Sysmodule * sysmodule();
            // Returns theme pointer
            Theme * theme();

            // Handles display loop
            void run();
            // Call to stop display loop (set true to force close)
            void exit(const bool);

            // Destructor frees memory and quits Aether
            ~Application();
    };
};

#endif