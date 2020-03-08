#include "Log.h"
#include <unistd.h>

// Path to log file
#define LOG_FILE "/switch/TriPlayer/sys-triplayer-log.txt"
// Path to log flag
#define LOG_FLAG "/config/TriPlayer/log.flag"

// Is logging enabled/available? (0 for true)
static int logEnabled = -1;

FILE * logOpenFile() {
    // Check if flag is present
    if (access(LOG_FLAG, F_OK) != -1) {
        logEnabled = 0;
    } else {
        logEnabled = -2;
        return NULL;
    }

    // Open log file if flag exists
    FILE * logFile = fopen(LOG_FILE, "w");
    if (logFile == NULL) {
        logEnabled = -3;
        return NULL;
    }

    return logFile;
}

void logCloseFile(FILE * f) {
    if (logEnabled != 0 || f == NULL) {
        return;
    }

    fclose(f);
}

void logError(FILE * f, const char * msg, int err) {
    if (logEnabled != 0 || f == NULL) {
        return;
    }

    fprintf(f, "[ERROR] %s: (%i)\n", msg, err);
    fflush(f);
}

void logSuccess(FILE * f, const char * msg) {
    if (logEnabled != 0 || f == NULL) {
        return;
    }

    fprintf(f, "[SUCCESS] %s\n", msg);
    fflush(f);
}

void logChar(FILE * f, const char c) {
    if (logEnabled != 0 || f == NULL) {
        return;
    }

    fprintf(f, "%c", c);
    fflush(f);
}