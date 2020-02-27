#ifndef AETHER_THREADED_HPP
#define AETHER_THREADED_HPP

#include <functional>
#include <future>

namespace Aether {
    enum class ThreadStatus {
        Created,    // Created but not started
        Running,    // Thread currently running
        Finished    // Thread finished execution
    };

    // This class only stores variables and methods relating to threads
    // and is "added-on" to elements using threading
    class Threaded {
        private:
            // Future
            std::future<void> future;
            // Function to call when the thread is started
            std::function<void()> threadFunc;
            // Status of thread
            ThreadStatus status_;

        public:
            // Constructor takes function to execute (but doesn't start!)
            Threaded(std::function<void()>);

            // Call to start execution
            void start();

            // Returns status of thread
            ThreadStatus status();
    };
};

#endif