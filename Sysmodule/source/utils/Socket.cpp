#include <arpa/inet.h>
#include <Commands.h>
#include <errno.h>
#include "Log.hpp"
#include "Socket.hpp"
#include <stdlib.h>
#include <string.h>
#include <switch.h>
#include <sys/socket.h>
#include <unistd.h>

// Read buffer grow size (in bytes)
#define BUFFER_SIZE 1000
// Message cache size (num of messages)
#define CACHE_SIZE 10
// Queue size
#define CONN_QUEUE 0
// Port to listen on
#define LISTEN_PORT 3333
// Timeout (sec) for accepts and reads
#define TIMEOUT 3
#define TIMEOUTE 1E+9

static struct sockaddr_in addr;
// Listening socket
static int lSocket = -1;
// Current transfer socket
static int tSocket = -1;

// Cache of received messages
// If this is full other messages will be lost
static char * msgCache[CACHE_SIZE] = {0};

int createListeningSocket() {
    // Get socket
    lSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (lSocket < 0) {
        Log::writeError("[SOCKET] Unable to create listening socket");
        return -1;
    }

    // Bind and listen
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(LISTEN_PORT);
    const int optVal = 1;
    setsockopt(lSocket, SOL_SOCKET, SO_REUSEADDR, (void*)&optVal, sizeof(optVal));
    if (bind(lSocket, (struct sockaddr*) &addr, sizeof(addr)) != 0) {
        Log::writeError("[SOCKET] Error binding to address/port");
        return -2;
    }
    if (listen(lSocket, CONN_QUEUE) != 0) {
        Log::writeError("[SOCKET] Error listening");
        return -3;
    }

    // Succeeded
    Log::writeSuccess("[SOCKET] Listening socket created successfully!");
    return 0;
}

void closeListeningSocket() {
    if (lSocket >= 0) {
        close(lSocket);
        lSocket = -1;
    }
}

void acceptConnection() {
    // Set timeout
    struct timeval time;
    time.tv_sec = TIMEOUT;
    time.tv_usec = 0;

    // Check for a ready connection
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(lSocket, &readfds);
    int error = 0;
    switch (select(lSocket + 1, &readfds, NULL, NULL, &time)) {
        case -1:
            Log::writeError("[SOCKET] Error occurred calling select()");
            // Sleep thread if select didn't pause
            svcSleepThread(TIMEOUTE);
            break;

        case 0:
            // Timed out
            break;

        default: {
            socklen_t sz = sizeof(addr);
            int tmp = accept(lSocket, (struct sockaddr *) &addr, &sz);
            if (tmp >= 0) {
                tSocket = tmp;

                // Set read timeout
                setsockopt(tSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)& time, sizeof(time));
                Log::writeSuccess("[SOCKET] Transfer socket connected!");
            } else {
                error = errno;
            }
            break;
        }
    }

    // Check error to determine if we just woke from sleep
    if (error == 113) {
        // This runs twice and I don't know why /shrug
        Log::writeWarning("[SOCKET] No route to host - normal behaviour after waking from sleep");
        Log::writeWarning("[SOCKET] Reinitializing all sockets");

        // Reinit sockets
        closeConnection();
        closeListeningSocket();
        socketExit();
        socketInitializeDefault();
        createListeningSocket();
    }
}

int haveConnection() {
    if (tSocket >= 0) {
        return 0;
    }

    return -1;
}

void closeConnection() {
    if (tSocket >= 0) {
        close(tSocket);
        tSocket = -1;
    }
}

char * readData() {
    // If there are no 'cached' messages read from socket
    if (msgCache[0] == NULL) {
        char * buf = NULL;  // Buffer to read into (will grow)
        int pos = 0;        // Position buffer finishes at
        int rd = 0;         // Number of bytes read in iteration

        // Read until at least one whole message is received
        // Even if there is another message waiting in socket buffer it will be handled on next call
        do {
            // (Create) Increase buffer size each time
            char * tmp = (char *) realloc((void *) buf, pos + BUFFER_SIZE);
            if (tmp == NULL) {
                // Error occurred increasing buffer size
                Log::writeError("[SOCKET] Unable to increase read buffer - out of memory?");
                free(buf);
                return NULL;
            }
            buf = tmp;
            rd = read(tSocket, buf + pos, BUFFER_SIZE);
            if (rd == 0) {
                // Lost connection while attempting to read
                closeConnection();
                Log::writeWarning("[SOCKET] Lost connection on read - closed tSocket");
                free(buf);
                return NULL;

            } else if (rd < 0) {
                // Timed out waiting to read
                free(buf);
                return NULL;
            }
            pos += rd;
        } while (buf[pos - 1] != '\0');

        // Split messages up (in case multiple messages are sent together)
        char * msg = buf;
        while ((msg - buf) < pos) {
            // Copy into own memory
            char * tmp = (char *) malloc((strlen(msg) + 1) * sizeof(char));
            strcpy(tmp, msg);

            // Insert into buffer (if full messages will be lost)
            short i = 0;
            while (i < CACHE_SIZE) {
                if (msgCache[i] == NULL) {
                    msgCache[i] = tmp;
                    break;
                }
                i++;
            }
            if (i == CACHE_SIZE) {
                Log::writeError("[SOCKET] Message cache size insuffient - lost some messages");
                free(tmp);
                break;
            }

            // Get next message
            msg = strchr(msg, '\0');
            msg++;
        }

        free(buf);
    }

    // Return message and shift other messages down the queue
    if (msgCache[0] != NULL) {
        char * msg = msgCache[0];
        for (int i = 0; i < CACHE_SIZE - 1; i++) {
            msgCache[i] = msgCache[i + 1];
        }
        msgCache[CACHE_SIZE - 1] = NULL;
        return msg;
    }

    // This shouldn't ever be reached unless some unknown error occurs
    Log::writeError("[SOCKET] Unknown error occurred reading from socket");
    return NULL;
}

void writeData(const char * data) {
    int len = strlen(data) + 1;
    if (write(tSocket, data, len) != len) {
        Log::writeError("[SOCKET] Error writing data");
    }
}