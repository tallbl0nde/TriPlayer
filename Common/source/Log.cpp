#include <atomic>
#include <ctime>
#include "Log.hpp"
#include <mutex>

// Log file pointer
static std::atomic<FILE *> file = nullptr;
// Log level
static std::atomic<Log::Level> level = Log::Level::None;
// Mutex to handle actually writing to file
static std::mutex mutex;

static void writeString(std::string msg) {
    if (file == nullptr) {
        return;
    }

    // Get timestamp
    std::scoped_lock<std::mutex> mtx(mutex);
    std::time_t time = std::time(nullptr);
    struct tm * t = std::localtime(&time);
    char buf[12];
    std::strftime(buf, sizeof(buf), "[%T] ", t);

    // Lock stream and write!
    fprintf(file, "%s%s\n", buf, msg.c_str());
    fflush(file);
}

namespace Log {
    Level loggingLevel() {
        return level;
    }

    void setLogLevel(Level l) {
        level = l;
    }

    std::string levelToString(const Level l) {
        std::string str = "?";
        switch (l) {
            case Log::Level::Info:
                str = "Info";
                break;

            case Log::Level::Success:
                str = "Success";
                break;

            case Log::Level::Warning:
                str = "Warning";
                break;

            case Log::Level::Error:
                str = "Error";
                break;

            case Log::Level::None:
                str = "None";
                break;

            default:
                break;
        }
        return str;
    }

    bool openFile(std::string f, Level l) {
        level = l;

        // Open log file
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