#ifndef TRIOVERLAY_HPP
#define TRIOVERLAY_HPP

#include "tesla.hpp"

// Forward declare database object
class Database;

// The main overlay class. Contains code to start/stop services and load the initial
// GUI frame. The frame loaded depends on whether the services started successfully.
class TriOverlay : public tsl::Overlay {
    private:
        Database * database;        // Database object passed to gui
        bool dbInitialized;         // Indicates whether database opened successfully
        bool triInitialized;        // Indicates whether TriPlayer initialized

    public:
        // Constructor initializes variables
        TriOverlay();

        // Initialize all services required by the overlay
        void initServices();

        // Stop any started services
        void exitServices();

        // Determine the GUI frame to show based on result of services
        std::unique_ptr<tsl::Gui> loadInitialGui();
};

#endif