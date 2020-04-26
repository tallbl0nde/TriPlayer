#include "Commands.h"
#include "Database.h"
#include "Log.h"
#include "MP3.h"
#include <pthread.h>
#include "Socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>

// Heap size:
// Sockets: ~3MB
// MP3: ~2.5MB
// DB: ~0.5MB
// Queue: 0.1MB
#define INNER_HEAP_SIZE 6 * 1024 * 1024

// Stuff that I probably shouldn't change
u32 __nx_applet_type = AppletType_None;
size_t nx_inner_heap_size = INNER_HEAP_SIZE;
char   nx_inner_heap[INNER_HEAP_SIZE];
void __libnx_initheap(void) {
	void*  addr = nx_inner_heap;
	size_t size = nx_inner_heap_size;

	// Newlib
	extern char* fake_heap_start;
	extern char* fake_heap_end;

	fake_heap_start = (char*)addr;
	fake_heap_end   = (char*)addr + size;
}
// End stuff

// Service statuses
int logOK;
int mp3OK;

// Init services on start
void __attribute__((weak)) __appInit(void) {
    if (R_FAILED(smInitialize())) {
        fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));
    }

    if (R_FAILED(fsInitialize())) {
        fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));
    }

    fsdevMountSdmc();

    logOK = logOpenFile();

    if (R_FAILED(socketInitializeDefault())) {
        logMessage("[SOCKET] Failed to initialize sockets!");
    }

    mp3OK = mp3Init();
}

// Close services on quit
void __attribute__((weak)) __appExit(void) {
    // In reverse order
    if (mp3OK == 0) {
        mp3Exit();
    }
    socketExit();
    if (logOK == 0) {
        logCloseFile();
    }
    fsdevUnmountAll();
    fsExit();
    smExit();
}

// Mutex for accessing mp3* functions (as they are not thread safe!)
static pthread_mutex_t mp3Mutex;

// TEMPORARY (used for testing atm)
static int songID = -1;

// Function used in one thread for handling sockets + commands
void * socketThread(void * args) {
    int * exit = (int *)args;

    // Create 'control' socket
    createListeningSocket();

    // Loop until 'exit' signal received
    while (*exit != 0) {
        // Try connecting if there is no active connection
        if (haveConnection() != 0) {
            // This has a timeout so will block the thread for short amount of time
            acceptConnection();
        } else {
            // Read also has a time out so it blocks too
            // Also closes connection on error
            const char * data = readData();
            if (data != NULL) {
                // Data received... parse command to determine what to do!
                char * reply = NULL;
                char * args;
                int cmd = strtol(data, &args, 10);
                args++;
                switch ((enum SM_Command) cmd) {
                    // Reply with version of protocol (sysmodule version is irrelevant)
                    case VERSION:
                        reply = (char *) malloc(2 * sizeof(char));
                        sprintf(reply, "%i", SM_PROTOCOL_VERSION);
                        break;

                    case RESUME:
                        pthread_mutex_lock(&mp3Mutex);
                        mp3Resume();
                        pthread_mutex_unlock(&mp3Mutex);
                        break;

                    case PAUSE:
                        pthread_mutex_lock(&mp3Mutex);
                        mp3Pause();
                        pthread_mutex_unlock(&mp3Mutex);
                        break;

                    case PREVIOUS:
                        //
                        break;

                    case NEXT:
                        //
                        break;

                    case GETVOLUME:
                        // Set precision to 3 digits (therefore max of 7 chars + \0)
                        reply = (char *) malloc(8 * sizeof(char));
                        sprintf(reply, "%.3lf", mp3Volume()*100);
                        break;

                    case SETVOLUME:
                        mp3SetVolume(strtod(args, NULL)/100.0);
                        break;

                    case PLAY: {
                        int len = strlen(data);

                        // Get path to song
                        if (len > 2) {
                            dbOpenReadOnly();
                            songID = strtol(args, NULL, 10);
                            const char * path = dbGetPath(songID);
                            dbClose();

                            // Play song
                            if (strlen(path) > 0) {
                                pthread_mutex_lock(&mp3Mutex);
                                mp3Stop();
                                mp3Play(path);
                                pthread_mutex_unlock(&mp3Mutex);
                            }

                            free((void *) path);
                        }
                        break;
                    }

                    case ADDTOQUEUE:

                        break;

                    case REMOVEFROMQUEUE:

                        break;

                    case GETQUEUE:

                        break;

                    case SETQUEUE:

                        break;

                    case SHUFFLE:

                        break;

                    case SETREPEAT:

                        break;

                    case GETSONG:
                        // 10 for int and 1 for \0
                        reply = (char *) malloc(11 * sizeof(char));
                        sprintf(reply, "%i", songID);
                        break;

                    case GETSTATUS:
                        // One byte for status + \0
                        reply = (char * ) malloc(2 * sizeof(char));
                        enum SM_Status s = ERROR;
                        pthread_mutex_lock(&mp3Mutex);
                        switch (mp3Status()) {
                            case Playing:
                            case Waiting:
                                s = PLAYING;
                                break;

                            case Paused:
                                s = PAUSED;
                                break;

                            case Stopped:
                                s = STOPPED;
                                break;
                        }
                        pthread_mutex_unlock(&mp3Mutex);
                        sprintf(reply, "%i", s);
                        break;

                    case GETPOSITION:
                        // Set precision to 5 digits (therefore max of 9 chars) + \0
                        reply = (char *) malloc(10 * sizeof(char));
                        pthread_mutex_lock(&mp3Mutex);
                        sprintf(reply, "%.5lf", mp3Position());
                        pthread_mutex_unlock(&mp3Mutex);
                        break;

                    case GETSHUFFLE:

                        break;

                    case GETREPEAT:

                        break;

                    case RESET:

                        break;
                }

                // Send reply if necessary
                if (reply != NULL) {
                    writeData(reply);
                    free((void *) reply);
                }

                // Delete received data
                free((void *) data);
            }
        }
    }

    // I don't think this will ever be required but it's here just in case
    closeConnection();

    // Close socket
    closeListeningSocket();

    return NULL;
}

int main(int argc, char * argv[]) {
    int exitThreads = -1;

    // Exit if unable to create a mutex
    if (pthread_mutex_init(&mp3Mutex, NULL) != 0) {
        return -1;
    }

    // Start socket thread (passed bool which is set 0 when to stop listening and exit thread)
    pthread_t pthreadSocket;
    pthread_create(&pthreadSocket, NULL, &socketThread, (void *) &exitThreads);

    // loop indefinitely
    while (appletMainLoop()) {
        // Outputs next 'section' of audio
        // Blocks this thread shortly (whether queuing audio or not)
        pthread_mutex_lock(&mp3Mutex);
        if (mp3OK == 0) {
            mp3Loop();
        }
        pthread_mutex_unlock(&mp3Mutex);

        // Flush log buffers to disk
        if (logOK == 0) {
            logProcess();
        }
    }

    // Join all threads
    exitThreads = 0;
    pthread_join(pthreadSocket, NULL);
    pthread_mutex_destroy(&mp3Mutex);

    return 0;
}