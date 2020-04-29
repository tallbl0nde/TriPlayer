#ifndef LOG_H
#define LOG_H

#include <string>

namespace Log {
    // Log levels (level set will log it and everything below)
    enum class Level {
        Info = 0,
        Success = 1,
        Warning = 2,
        Error = 3
    };

    // Open file for writing (must be called in writing thread)
    bool openFile(Level = Level::Info);

    // Log message
    void writeError(std::string);
    void writeInfo(std::string);
    void writeSuccess(std::string);
    void writeWarning(std::string);
}

#endif