#ifndef GUI_PLAYER_HPP
#define GUI_PLAYER_HPP

#include "tesla.hpp"

// Forward declarations
class Database;
namespace Element {
    class Player;
};

// The Player gui represents the main player controls frame.
// It contains a single Player element, which in turn presents
// buttons and information.
namespace Gui {
    class Player : public tsl::Gui {
        private:
            Database * database;        // Database used to read metadata from
            Element::Player * player;   // Main element
            unsigned int ticks;         // Number of ticks in update() since last check

            int currentSongID;          // ID of song matching stored metadata

        public:
            // Initialize objects
            Player(Database *);

            // Accepts database object to read metadata from
            tsl::elm::Element * createUI();

            // Periodically check if we need to update the element
            void update();
    };
};

#endif