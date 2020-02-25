#ifndef SCREEN_SPLASH_HPP
#define SCREEN_SPLASH_HPP

#include "Application.hpp"

namespace Main {
    class Application;
};

namespace Screen {
    // Splash/loading screen
    class Splash : public Aether::Screen {
        private:
            Main::Application * app;

            // Elements
            Aether::Text * name;
            Aether::Text * status;
            Aether::RoundProgressBar * pbar;
            Aether::Text * pbartext;

        public:
            Splash(Main::Application *);

            // onLoad creates all elements
            void onLoad();

            // onUnload deletes all elements
            void onUnload();
    };
};

#endif