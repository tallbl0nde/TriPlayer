#include <chrono>
#include <thread>
#include "Threaded.hpp"

namespace Aether {
    Threaded::Threaded(std::function<void()> f) {
        this->threadFunc = f;
        this->status_ = ThreadStatus::Created;
    }

    void Threaded::start() {
        if (this->status() != ThreadStatus::Running) {
            this->future = std::async(std::launch::async, this->threadFunc);
        //     this->status_ = ThreadStatus::Running;
        }
    }

    ThreadStatus Threaded::status() {
        if (this->status_ != ThreadStatus::Finished) {
            if (this->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                this->status_ = ThreadStatus::Finished;
            }
        }

        return this->status_;
    }
};