#ifndef UTILS_TIMER_HPP
#define UTILS_TIMER_HPP

#include <chrono>

namespace Utils {
    // A basic Timer class to time actions, etc
    class Timer {
        private:
            // Start + end times
            std::chrono::time_point<std::chrono::steady_clock> startTime;
            std::chrono::time_point<std::chrono::steady_clock> endTime;
            bool isRunning;

        public:
            // The constructor doesn't start the timer
            Timer();

            // Start the timer (does nothing if running)
            void start();

            // Stop the timer (does nothing if running)
            void stop();

            // Return the number of milliseconds since timer was started if the timer is running
            // or the number of milliseconds elapsed while it was running
            double elapsedMillis();

            // Same as above but returns seconds
            double elapsedSeconds();
    };
};

#endif