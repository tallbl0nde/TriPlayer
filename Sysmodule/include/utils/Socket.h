#ifndef SOCKET_H
#define SOCKET_H

// Essentially a socket 'class'

// Create listening socket (returns 0 on success)
int createListeningSocket();
// Closes listening socket
void closeListeningSocket();

// Wait for a connection before timing out
void acceptConnection();
// Returns 0 if the transfer socket is connected
int haveConnection();
// Close accepted connection
void closeConnection();

// Attempt to read data before timing out (returns pointer to malloc'd data - so free!!)
// Returns NULL if timedout
char * readData();

// Write data to the socket (doesn't free string!)
void writeData(const char *);

#endif