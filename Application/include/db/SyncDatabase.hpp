
#ifndef SYNCDATABASE_HPP
#define SYNCDATABASE_HPP

#include "db/Database.hpp"
#include <functional>
#include <memory>
#include <mutex>

// This wraps my Database class methods within a mutex using the 'Execute-Around' pattern
// The code that helped me write this can be found under the MIT license
// here: https://github.com/ArnaudBienner/ExecuteAround
class SyncDatabase {
    private:
        class SyncDatabaseProxy {
            private:
                // Database pointer to invoke methods on
                Database * ptr;

                // Function to call before invoking method
                std::function<void()> beforeFunc;
                // Function to call after invoking method
                std::function<void()> afterFunc;

            public:
                // Constructor locks the mutex
                SyncDatabaseProxy(Database *, std::function<void()>, std::function<void()>);

                // Destructor unlocks the mutex
                ~SyncDatabaseProxy();

                // Overload the -> operator to return the stored pointer
                const Database * operator->() const;
                Database * operator->();
        };

        // Database pointer to pass to proxy
        std::shared_ptr<Database> ptr;
        // Mutex to lock
        mutable std::mutex mutex;

    public:
        // Constructor simply stores the pointer to invoke methods on
        SyncDatabase(Database *);

        // Default constructor included as I need to store references
        SyncDatabase();

        // Override -> operator to invoke the before/after methods
        SyncDatabaseProxy operator->() const;
};

#endif