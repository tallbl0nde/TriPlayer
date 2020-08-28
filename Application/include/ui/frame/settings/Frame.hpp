#ifndef FRAME_SETTINGS_FRAME
#define FRAME_SETTINGS_FRAME

#include "Aether/Aether.hpp"

namespace Main {
    class Application;
};

namespace Frame::Settings {
    // 'Template' class which is inherited to create certain
    // types of frames. It provides some helpers to easily
    // add entries to the list.
    class Frame : public Aether::Container {
        protected:
            // List of elements
            Aether::List * list;

            // Pointer to main object
            Main::Application * app;

            // Add a 'yes/no' list entry
            // First arg is string to show
            // Second arg is function to call to check value
            // Third arg is function to call to set new value
            void addToggle(const std::string &, std::function<bool()>, std::function<void(bool)>);
            // Add a comment to the list
            void addComment(const std::string &);

            // Open the numpad and get a numeric input
            // Returns whether successful or not
            bool getNumberInput(int &, const std::string &, const std::string &);

        public:
            // Constructor takes pointer to app object and creates list
            Frame(Main::Application *);
    };
};

#endif