#ifndef SCREEN_SCREEN_HPP
#define SCREEN_SCREEN_HPP

#include "Aether/Aether.hpp"

namespace Main {
    class Application;
};

// Inherits Aether::Screen and adds a few common methods
// that this app's screens will use
namespace Screen {
    class Screen : public Aether::Screen {
        protected:
            // Main app pointer
            Main::Application * app;

            // Set true when screen is 'loaded'
            bool isLoaded;

        public:
            // Constructor accepts pointer to application object
            Screen(Main::Application *);

            // Called to update colours used without the need of recreation
            virtual void updateColours();

            // Sets isLoaded bool
            void onLoad();
            void onUnload();
    };
};

#endif