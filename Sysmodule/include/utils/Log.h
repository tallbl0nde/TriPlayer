#ifndef LOG_H
#define LOG_H

// Handles all logging activities (thread-safe thanks to mutexes!)
// Each log message is added to a queue and then deleted (if specified) once logged

// Open file for writing (must be called in writing thread)
int logOpenFile();
// Close file (must be called in thread that called open)
void logCloseFile();

// Log message (w/o deleting string)
void logMessage(char *);
// Log message (and delete string when done)
void logMessageAndFree(char *);

// Flush buffers and write to disk (must call in thread that opened)
void logProcess();

#endif