#ifndef OVERLAY_SORTBY_HPP
#define OVERLAY_SORTBY_HPP

#include "db/Database.hpp"
#include "ui/overlay/Overlay.hpp"

namespace CustomElm {
    class MenuButton;
};

namespace CustomOvl {
    // Overlay which presents a list of sorting options. These options can
    // be configured upon create of the object.
    // The chosen option will be passed to the provided callback.
    class SortBy : public Overlay {
        public:
            struct Entry {
                Database::SortBy type;                  // Entry type
                std::string text;                       // Entry string
            };

        private:
            Aether::Rectangle * bg;                     // Background rectangle
            Aether::Text * heading;                     // Heading
            Aether::Rectangle * line;                   // Separator line
            Aether::List * list;                        // List of buttons
            std::vector<CustomElm::MenuButton *> btns;  // Actual button elements used to change colour

            // Button callback
            std::function<void(Database::SortBy)> callback;

        public:
            // Create the overlay, using titlea and buttons in the same order
            SortBy(const std::string &, const std::vector<Entry> &, std::function<void(Database::SortBy)>);

            // Set element colours
            void setBackgroundColour(const Aether::Colour &);
            void setIconColour(const Aether::Colour &);
            void setLineColour(const Aether::Colour &);
            void setTextColour(const Aether::Colour &);
    };
};

#endif