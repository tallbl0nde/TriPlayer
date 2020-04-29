#include <future>
#include "Log.hpp"
#include "Service.hpp"

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
    MP3::initLib();
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

int main(int argc, char * argv[]) {
    // Create Service
    MainService * s = new MainService();

    // Start communication thread
    std::future<void> socketThread = std::async(std::launch::async, &MainService::process, s);
    // Start decoding thread
    std::future<void> decodeThread = std::async(std::launch::async, &MainService::decodeSource, s);

    // This thread is responsible for managing audio output + buffers
    audio->process();

    // Join threads (only run after audio is done)
    s->exit();
    decodeThread.get();
    socketThread.get();

    // Now that it's done we can delete!
    delete s;

    return 0;
}