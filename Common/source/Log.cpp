#include <ctime>
#include "Log.hpp"
#include <mutex>

// Path to log flag
#define LOG_FLAG "/config/TriPlayer/log.flag"

// Log file pointer
static FILE * file;
// Log level
static Log::Level level;
// Mutex to handle actually writing to file
static std::mutex mutex;

static void writeString(std::string msg) {
    if (file == nullptr) {
        return;
    }

    // Get timestamp
    std::time_t time = std::time(nullptr);
    struct tm * t = std::localtime(&time);
    char buf[12];
    std::strftime(buf, sizeof(buf), "[%T] ", t);

    // Lock stream and write!
    std::lock_guard<std::mutex> mtx(mutex);
    fprintf(file, "%s%s\n", buf, msg.c_str());
    fflush(file);
}

namespace Log {
    Level loggingLevel() {
        return level;
    }

    bool openFile(std::string f, Level l) {
        level = l;

        // Check if flag is present
        if (access(f.c_str(), F_OK) == -1) {
            return false;
        }

        // Open log file if flag exists
        file = fopen(f.c_str(), "a");
        if (file == nullptr) {
            return false;
        }

        return true;
    }

    void closeFile() {
        if (file != nullptr) {
            fclose(file);
            file = nullptr;
        }
    }

    void writeError(std::string msg) {
        if (level <= Level::Error) {
            writeString("[E] " + msg);
        }
    }
    void writeInfo(std::string msg) {
        if (level <= Level::Info) {
            writeString("[I] " + msg);
        }
    }
    void writeSuccess(std::string msg) {
        if (level <= Level::Success) {
            writeString("[S] " + msg);
        }
    }
    void writeWarning(std::string msg) {
        if (level <= Level::Warning) {
            writeString("[W] " + msg);
        }
    }
}