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
            Aether::Image * bg;
            Aether::Text * status;
            Aether::Text * statusNum;
            Aether::RoundProgressBar * pbar;
            Aether::Text * percent;
            Aether::Animation * anim;
            Aether::Text * hint;

        public:
            Splash(Main::Application *);

            // onLoad creates all elements
            void onLoad();

            // onUnload deletes all elements
            void onUnload();
    };
};

#endif