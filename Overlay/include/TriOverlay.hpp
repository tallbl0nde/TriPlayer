#ifndef TRIOVERLAY_HPP
#define TRIOVERLAY_HPP

#include "tesla.hpp"

// The main overlay class. Contains code to start/stop services and load the initial
// GUI frame. The frame loaded depends on whether the services started successfully.
class TriOverlay : public tsl::Overlay {
    private:
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