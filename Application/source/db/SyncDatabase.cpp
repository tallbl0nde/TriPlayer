#include "db/SyncDatabase.hpp"

SyncDatabase::SyncDatabaseProxy::SyncDatabaseProxy(Database * ptr, std::function<void()> before, std::function<void()> after) {
    this->ptr = ptr;
    this->beforeFunc = before;
    this->afterFunc = after;
    this->beforeFunc();
}

SyncDatabase::SyncDatabaseProxy::~SyncDatabaseProxy() {
    this->afterFunc();
}

const Database * SyncDatabase::SyncDatabaseProxy::operator->() const {
    return this->ptr;
}

Database * SyncDatabase::SyncDatabaseProxy::operator->() {
    return this->ptr;
}

SyncDatabase::SyncDatabase(Database * ptr) {
    this->ptr = std::shared_ptr<Database>(ptr);
}

SyncDatabase::SyncDatabase() {
    this->ptr = nullptr;
}

SyncDatabase::SyncDatabaseProxy SyncDatabase::operator->() const {
    return SyncDatabase::SyncDatabaseProxy(this->ptr.get(), [this]() {
        mutex.lock();
    }, [this]() {
        mutex.unlock();
    });
}