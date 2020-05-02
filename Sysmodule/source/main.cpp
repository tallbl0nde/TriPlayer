#include <future>
#include "Log.hpp"
#include "Service.hpp"

// Path to log file
#define LOG_FILE "/switch/TriPlayer/sysmodule.log"

// Heap size:
// Sockets: ~0.1MB
// MP3: ~0.5MB
// DB: ~0.5MB
#define INNER_HEAP_SIZE 1 * 1024 * 1024

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

    Log::openFile(LOG_FILE, Log::Level::Success);
    Log::writeSuccess("=== Sysmodule started! ===");

    // Sockets use small buffers
    constexpr SocketInitConfig sockCfg = {
        .bsdsockets_version = 1,

        .tcp_tx_buf_size = 0x1000,
        .tcp_rx_buf_size = 0x1000,
        .tcp_tx_buf_max_size = 0x3000,
        .tcp_rx_buf_max_size = 0x3000,

        .udp_tx_buf_size = 0x0,
        .udp_rx_buf_size = 0x0,

        .sb_efficiency = 1,
    };
    if (R_FAILED(socketInitialize(&sockCfg))) {
        Log::writeError("[SOCKET] Failed to initialize sockets!");
    }

    // Audio
    audrenInitialize(&audrenCfg);
    Audio::getInstance();
    audrenStartAudioRenderer();
    MP3::initLib();
}

// Close services on quit
void __appExit(void) {
    // In reverse order

    // Audio
    audrenStopAudioRenderer();
    delete Audio::getInstance();
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
    Audio::getInstance()->process();

    // Join threads (only run after audio is done)
    s->exit();
    decodeThread.get();
    socketThread.get();

    // Now that it's done we can delete!
    delete s;

    return 0;
}