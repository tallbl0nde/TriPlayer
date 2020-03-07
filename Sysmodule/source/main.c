#include "Log.h"
#include "MP3.h"
#include <pthread.h>
#include "Socket.h"
#include <stdlib.h>
#include <string.h>
#include <switch.h>

// Heap size... 3MB for sockets + 3MB for audio (atm)
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

// Function used in one thread for handling music
void * musicThread(void * args) {
    int * exit = (int *)args;

logOpenFile();
    mp3Init();

    mp3Play("/music/3nd of an 3ra.mp3");

    while (true) {
        mp3Loop();
    }

    mp3Exit();
    logCloseFile();

    return NULL;
}

// Function used in one thread for handling sockets + commands
void * socketThread(void * args) {
    int * exit = (int *)args;

    // Start logging

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
                // Data received... do something!
                logSuccess("Received some data in main!");


                free((void *)data);
            }
        }
    }

    // I don't think this will ever be required but it's here just in case
    closeConnection();

    // Stop logging

    // Close socket
    closeListeningSocket();

    return NULL;
}

int main(int argc, char * argv[]) {
    int exitThreads = -1;

    // Start socket thread (passed bool which is set 0 when to stop listening and exit thread)
    // pthread_t pthreadSocket;
    // pthread_create(&pthreadSocket, NULL, &socketThread, (void *) &exitThreads);

    // Music player (mpg123) thread
    pthread_t pthreadMusic;
    pthread_create(&pthreadMusic, NULL, &musicThread, (void *) &exitThreads);

    // loop indefinitely
    while (appletMainLoop()) {
        svcSleepThread(1E+9);
    }

    // Join all threads
    exitThreads = 0;
    pthread_join(pthreadMusic, NULL);
    // pthread_join(pthreadSocket, NULL);

    return 0;
}