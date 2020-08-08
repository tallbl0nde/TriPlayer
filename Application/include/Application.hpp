#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <future>
#include "db/Database.hpp"
#include "sysmodule/Sysmodule.hpp"
#include "ui/Theme.hpp"

// Forward declaration because cyclic dependency /shrug
namespace Screen {
    class Home;
    class Splash;
    class Fullscreen;
};

namespace Main {
    // Enumeration for screens (allows for easy switching)
    enum ScreenID {
        Home,
        Splash,
        Fullscreen
    };

    // The Application class represents the "root" object of the app. It stores/handles all states
    // and objects used through the app
    class Application {
        private:
            // Display object used for rendering
            Aether::Display * display;

            // Screens of the app
            Screen::Fullscreen * scFull;
            Screen::Home * scHome;
            Screen::Splash * scSplash;

            // Database object
            Database * database_;

            // Sysmodule object which allows communication
            Sysmodule * sysmodule_;

            // Theme object
            Theme * theme_;

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

            // Pass screen enum to change to it
            void setScreen(ScreenID);
            // Push current screen on stack (i.e. keep in memory)
            void pushScreen();
            // Pop screen from stack
            void popScreen();

            // Helper functions for database
            void lockDatabase();
            void unlockDatabase();

            // Returns database pointer
            Database * database();
            // Returns sysmodule pointer
            Sysmodule * sysmodule();
            // Returns theme pointer
            Theme * theme();

            // Handles display loop
            void run();
            // Call to stop display loop
            void exit();

            // Destructor frees memory and quits Aether
            ~Application();
    };
};

#endif