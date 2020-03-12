#include "Commands.h"
#include "Database.h"
#include "MP3.h"
#include <pthread.h>
#include "Socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>

// Heap size... 3MB for sockets + 3MB for audio (atm)
#define INNER_HEAP_SIZE (3 + 3) * 1024 * 1024

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

// Init services on start
void __attribute__((weak)) __appInit(void) {
    if (R_FAILED(smInitialize())) {
        fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));
    }

    if (R_FAILED(fsInitialize())) {
        fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));
    }

    fsdevMountSdmc();

    if (R_FAILED(socketInitializeDefault())) {
        // I dunno
    }

    if (R_FAILED(audoutInitialize())) {
        // I dunno Part 2
    }
    audoutStartAudioOut();
}

// Close services on quit
void __attribute__((weak)) __appExit(void) {
    // In reverse order
    audoutStopAudioOut();
    audoutExit();
    socketExit();
    fsdevUnmountAll();
    fsExit();
    smExit();
}

// Mutex for accessing mp3* functions (as they are not thread safe!)
static pthread_mutex_t mp3Mutex;

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
                switch ((enum SM_Command) (data[0] - '0')) {
                    // Reply with version of protocol (sysmodule version is irrelevant)
                    case VERSION:
                        reply = (char *) malloc(2 * sizeof(char));
                        sprintf(reply, "%i", SM_PROTOCOL_VERSION);
                        break;

                    case PLAY: {
                        int len = strlen(data);

                        // Get path to song
                        if (len > 2) {
                            dbOpenReadOnly();
                            char id[(sizeof(int) * 8) + 1];
                            len = strlen(&data[2]);
                            strcpy(id, &data[2]);
                            const char * path = dbGetPath(strtol(id, &id, 10));
                            dbClose();

                            // Play song
                            if (strlen(path) > 0) {
                                pthread_mutex_lock(&mp3Mutex);
                                mp3Stop();
                                mp3Play(path);
                                pthread_mutex_unlock(&mp3Mutex);
                            }

                            free(path);
                        }
                        break;
                    }

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

    // Prepare mp3 stuff
    mp3Init();

    // Start socket thread (passed bool which is set 0 when to stop listening and exit thread)
    pthread_t pthreadSocket;
    pthread_create(&pthreadSocket, NULL, &socketThread, (void *) &exitThreads);

    // loop indefinitely
    while (appletMainLoop()) {
        // Outputs next 'section' of audio
        // Blocks this thread shortly (whether queuing audio or not)
        pthread_mutex_lock(&mp3Mutex);
        mp3Loop();
        pthread_mutex_unlock(&mp3Mutex);
    }

    // Join all threads
    exitThreads = 0;
    pthread_join(pthreadSocket, NULL);

    // Cleanup mp3 stuff
    mp3Exit();

    pthread_mutex_destroy(&mp3Mutex);

    return 0;
}