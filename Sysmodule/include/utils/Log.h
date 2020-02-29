#ifndef LOG_H
#define LOG_H

// Handles all logging activities

// Open file for writing
void logOpenFile();
// Close file
void logCloseFile();

// Log message with [ERROR]
void logError(const char *, int);
// Log message with [SUCCESS]
void logSuccess(const char *);

void logChar(const char);

#endif