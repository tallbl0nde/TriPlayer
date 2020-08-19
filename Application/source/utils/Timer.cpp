#include "utils/Timer.hpp"

namespace Utils {
    Timer::Timer() {
        this->isRunning = false;
    }

    void Timer::start() {
        this->startTime = std::chrono::steady_clock::now();
        this->isRunning = true;
    }

    void Timer::stop() {
        this->endTime = std::chrono::steady_clock::now();
        this->isRunning = false;
    }

    double Timer::elapsedMillis() {
        if (this->isRunning) {
            std::chrono::time_point<std::chrono::steady_clock> now;
            now = std::chrono::steady_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(now - this->startTime).count();

        } else {
            return std::chrono::duration_cast<std::chrono::milliseconds>(this->endTime - this->startTime).count();
        }
    }

    double Timer::elapsedSeconds() {
        return this->elapsedMillis()/1000.0;
    }
};