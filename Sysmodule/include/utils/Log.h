#ifndef LOG_H
#define LOG_H

#include <stdio.h>

// Handles all logging activities

// Open file for writing
FILE * logOpenFile();
// Close file
void logCloseFile(FILE *);

// Log message with [ERROR]
void logError(FILE *, const char *, int);
// Log message with [SUCCESS]
void logSuccess(FILE *, const char *);

void logChar(FILE *, const char);

#endif