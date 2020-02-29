#include "Log.h"
#include <stdio.h>
#include <unistd.h>

// Path to log file
#define LOG_FILE "/switch/TriPlayer/sys-triplayer-log.txt"
// Path to log flag
#define LOG_FLAG "/config/TriPlayer/log.flag"

// Is logging enabled/available? (0 for true)
static int logEnabled = -1;
// File pointer
static FILE * logFile;

void logOpenFile() {
    // Check if flag is present
    if (access(LOG_FLAG, F_OK) != -1) {
        logEnabled = 0;
    } else {
        logEnabled = -2;
        return;
    }

    // Open log file if flag exists
    logFile = fopen(LOG_FILE, "w");
    if (logFile == NULL) {
        logEnabled = -3;
    }
}

void logCloseFile() {
    if (logEnabled != 0) {
        return;
    }

    fclose(logFile);
}

void logError(const char * msg, int err) {
    if (logEnabled != 0) {
        return;
    }

    fprintf(logFile, "[ERROR] %s: (%i)\n", msg, err);
    fflush(logFile);
}

void logSuccess(const char * msg) {
    if (logEnabled != 0) {
        return;
    }

    fprintf(logFile, "[SUCCESS] %s\n", msg);
    fflush(logFile);
}

void logChar(const char c) {
    if (logEnabled != 0) {
        return;
    }

    fprintf(logFile, "%c", c);
    fflush(logFile);
}