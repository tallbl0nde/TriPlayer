#include "Audio.hpp"
// #include "Commands.h"
// #include <cstring>
#include "Database.hpp"
#include <future>
#include "Log.hpp"
#include "MP3.hpp"
// #include "Socket.hpp"

// Heap size:
// Sockets: ~3MB
// MP3: ~0.5MB
// DB: ~0.5MB
#define INNER_HEAP_SIZE 4 * 1024 * 1024

// It hangs if I don't use C... I wish I knew why!
extern "C" {
    u32 __nx_applet_type = AppletType_None;

    size_t nx_inner_heap_size = INNER_HEAP_SIZE;
    char   nx_inner_heap[INNER_HEAP_SIZE];

    void __libnx_initheap(void);
    void __appInit(void);
    void __appExit(void);
}

void __libnx_initheap(void) {
    void*  addr = nx_inner_heap;
    size_t size = nx_inner_heap_size;

    // Newlib
    extern char* fake_heap_start;
    extern char* fake_heap_end;

    fake_heap_start = (char*)addr;
    fake_heap_end   = (char*)addr + size;
}

// Audio object
Audio * audio;

// Init services on start
void __appInit(void) {
    if (R_FAILED(smInitialize())) {
        fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));
    }

    // FS + Log
    if (R_FAILED(fsInitialize())) {
        fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));
    }
    fsdevMountSdmc();

    Log::openFile(Log::Level::Success);

    if (R_FAILED(socketInitializeDefault())) {
        Log::writeError("[SOCKET] Failed to initialize sockets!");
    }

    // Audio
    audrenInitialize(&audrenCfg);
    audio = Audio::getInstance();
    audrenStartAudioRenderer();
}

// Close services on quit
void __appExit(void) {
    // In reverse order

    // Audio
    audrenStopAudioRenderer();
    delete audio;
    audrenExit();

    // Socket
    socketExit();

    // FS
    fsdevUnmountAll();
    fsExit();
    smExit();
}

// TEMPORARY (used for testing atm)
// static int songID = -1;

// MP3 * source;


// // Function used in one thread for handling sockets + commands
// void socketThread(int * exit) {
//     // Create 'control' socket
//     createListeningSocket();

//     // Loop until 'exit' signal received
//     while (*exit != 0) {
//         // Try connecting if there is no active connection
//         if (haveConnection() != 0) {
//             // This has a timeout so will block the thread for short amount of time
//             acceptConnection();
//         } else {
//             // Read also has a time out so it blocks too
//             // Also closes connection on error
//             const char * data = readData();
//             if (data != NULL) {
//                 // Data received... parse command to determine what to do!
//                 char * reply = NULL;
//                 char * args;
//                 int cmd = strtol(data, &args, 10);
//                 args++;
//                 switch ((enum SM_Command) cmd) {
//                     // Reply with version of protocol (sysmodule version is irrelevant)
//                     case VERSION:
//                         reply = (char *) malloc(2 * sizeof(char));
//                         sprintf(reply, "%i", SM_PROTOCOL_VERSION);
//                         break;

//                     case RESUME:
//                         audio->resume();
//                         break;

//                     case PAUSE:
//                         audio->pause();
//                         break;

//                     case PREVIOUS:
//                         //
//                         break;

//                     case NEXT:
//                         //
//                         break;

//                     case GETVOLUME:
//                         // Set precision to 3 digits (therefore max of 7 chars + \0)
//                         reply = (char *) malloc(8 * sizeof(char));
//                         sprintf(reply, "%.3lf", audio->volume()*100);
//                         break;

//                     case SETVOLUME:
//                         // audio->setVolume(strtod(args, NULL)/100.0);
//                         break;

//                     case PLAY: {
//                         int len = strlen(data);

//                         // Get path to song
//                         if (len > 2) {
//                             // dbOpenReadOnly();
//                             // songID = strtol(args, NULL, 10);
//                             // const char * path = dbGetPath(songID);
//                             // dbClose();

//                             // // Play song
//                             // if (strlen(path) > 0) {
//                             //     pthread_mutex_lock(&mp3Mutex);
//                             //     mp3Stop();
//                             //     mp3Play(path);
//                             //     pthread_mutex_unlock(&mp3Mutex);
//                             // }

//                             // free((void *) path);
//                         }
//                         break;
//                     }

//                     case ADDTOQUEUE:

//                         break;

//                     case REMOVEFROMQUEUE:

//                         break;

//                     case GETQUEUE:

//                         break;

//                     case SETQUEUE:

//                         break;

//                     case SHUFFLE:

//                         break;

//                     case SETREPEAT:

//                         break;

//                     case GETSONG:
//                         // 10 for int and 1 for \0
//                         reply = (char *) malloc(11 * sizeof(char));
//                         sprintf(reply, "%i", songID);
//                         break;

//                     case GETSTATUS: {
//                         // One byte for status + \0
//                         reply = (char * ) malloc(2 * sizeof(char));
//                         SM_Status s = ERROR;
//                         switch (audio->status()) {
//                             case AudioStatus::Playing:
//                                 s = PLAYING;
//                                 break;

//                             case AudioStatus::Paused:
//                                 s = PAUSED;
//                                 break;

//                             case AudioStatus::Stopped:
//                                 s = STOPPED;
//                                 break;
//                         }
//                         sprintf(reply, "%i", s);
//                         break;
//                     }

//                     case GETPOSITION:
//                         // Set precision to 5 digits (therefore max of 9 chars) + \0
//                         reply = (char *) malloc(10 * sizeof(char));
//                         sprintf(reply, "%.5lf", source->position());
//                         break;

//                     case GETSHUFFLE:

//                         break;

//                     case GETREPEAT:

//                         break;

//                     case RESET:

//                         break;
//                 }

//                 // Send reply if necessary
//                 if (reply != NULL) {
//                     writeData(reply);
//                     free((void *) reply);
//                 }

//                 // Delete received data
//                 free((void *) data);
//             }
//         }
//     }

//     // I don't think this will ever be required but it's here just in case
//     closeConnection();

//     // Close socket
//     closeListeningSocket();
// }

MP3 * source;

void decodeThread(int * exit) {
    Database * db = new Database();
    std::string path = db->getPathForID(2);
    delete db;
    source = new MP3(path);
    audio->newSong(source->sampleRate(), source->channels());
    while (source->valid()) {
        u8 * buf = new u8[audio->bufferSize()];
        size_t dec = source->decode(buf, audio->bufferSize());
        while (!audio->bufferAvailable()) {
            svcSleepThread(2E+7);
        }
        audio->addBuffer(buf, dec);
        delete[] buf;
    }
    delete source;
    audio->exit();
}

int main(int argc, char * argv[]) {
    int exitThreads = -1;

    MP3::initLib();

    // Start socket thread (passed bool which is set 0 when to stop listening and exit thread)
    // std::future<void> socketT = std::async(std::launch::async, &socketThread, &exitThreads);
    std::future<void> decodeT = std::async(std::launch::async, &decodeThread, &exitThreads);

    // loop indefinitely
    audio->process();

    // Join all threads
    exitThreads = 0;
    return 0;
}