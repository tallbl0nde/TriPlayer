#include "Log.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Path to log file
#define LOG_FILE "/switch/TriPlayer/sys-triplayer-log.txt"
// Path to log flag
#define LOG_FLAG "/config/TriPlayer/log.flag"
// Max size of log queue (if the queue is full messages will be lost)
#define QUEUE_SIZE 10

// Is logging enabled/available? (0 for true)
static int logEnabled = -1;
// File pointer
static FILE * f = NULL;

// 'Queue' of strings to log
static char * queue[QUEUE_SIZE];
static int queueSize = 0;

static pthread_mutex_t mutex;

int logOpenFile() {
    f = NULL;

    // Check if flag is present
    if (access(LOG_FLAG, F_OK) != -1) {
        logEnabled = 0;
    } else {
        return -1;
    }

    // Open log file if flag exists
    f = fopen(LOG_FILE, "a");
    if (f == NULL) {
        return -2;
    }

    // Create mutex
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        return -3;
    }

    fprintf(f, "\n");
    logMessage("sys-triplayer started!");
    return 0;
}

void logCloseFile() {
    if (f == NULL) {
        return;
    }

    pthread_mutex_destroy(&mutex);
    fclose(f);
}

void logMessage(char * msg) {
    if (f == NULL) {
        return;
    }

    // Add to queue if there is room
    pthread_mutex_lock(&mutex);
    if (queueSize < QUEUE_SIZE) {
        queue[queueSize] = msg;
        queueSize++;
    }
    pthread_mutex_unlock(&mutex);
}

void logMessageAndFree(char * msg) {
    // Don't add if no room
    pthread_mutex_lock(&mutex);
    if (queueSize >= QUEUE_SIZE) {
        free((void *) msg);
    }
    pthread_mutex_unlock(&mutex);

    // Change null terminator to 'end of text'
    // Indicates it should be deleted
    int len = strlen(msg);
    msg[len] = '\3'; 
    logMessage(msg);
}

void logProcess() {
    if (f == NULL) {
        return;
    }

    // Loop over queue
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < queueSize; i++) {
        // Fix message if it's to be deleted
        int delete = -1;
        int len = strlen(queue[i]);
        if (queue[i][len] == '\3') {
            queue[i][len] = '\0';
            delete = 0;
        }

        // Log time first
        time_t ti = time(NULL);
        struct tm * t = localtime(&ti);
        char buf[10];
        strftime(buf, sizeof(buf), "%T", t);
        fprintf(f, "[%s] ", buf);

        // Write message to file
        fprintf(f, "%s\n", queue[i]);

        // Delete
        if (delete == 0) {
            free((void *) queue[i]);
        }
    }
    queueSize = 0;
    pthread_mutex_unlock(&mutex);

    fflush(f);
}