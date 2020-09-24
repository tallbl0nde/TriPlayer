#include "nx/Audio.hpp"
#include "nx/NX.hpp"
#include "Service.hpp"
#include "sources/MP3.hpp"
#include <switch.h>

// Heap size:
// DB: ~0.5MB
// IPC: ~0.2MB
// Queue: ~0.2MB
// MP3: ~0.3MB
#define INNER_HEAP_SIZE (size_t)(2 * 1024 * 1024)

// It hangs if I don't use C... I wish I knew why!
extern "C" {
    u32 __nx_applet_type = AppletType_None;
    u32 __nx_fs_num_sessions = 1;

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

void __appInit(void) {
    // Initialize required services
    NX::startServices();

    // Prepare audio libraries
    MP3::initLib();
}

void __appExit(void) {
    // Clean up audio libraries first
    MP3::freeLib();

    // Finally stop services
    NX::stopServices();
}

// Wrappers to call methods on MainService object
void audioThread(void * arg) {
    static_cast<Audio *>(arg)->process();
}

void serviceGpioThread(void * arg) {
    static_cast<MainService *>(arg)->gpioEventThread();
}

void serviceIpcThread(void * arg) {
    static_cast<MainService *>(arg)->ipcThread();
}

void servicePowerThread(void * arg) {
    static_cast<MainService *>(arg)->sleepEventThread();
}

int main(int argc, char * argv[]) {
    // Create Service
    MainService * service = new MainService();

    // Spawn threads
    NX::Thread::create("audio", audioThread, Audio::getInstance());
    NX::Thread::create("gpio", serviceGpioThread, service);
    NX::Thread::create("ipc", serviceIpcThread, service);
    NX::Thread::create("power", servicePowerThread, service);

    // Use this thread to handle playback (we need the higher priority!)
    service->playbackThread();

    // Join threads (only executed after service has exit signal)
    Audio::getInstance()->exit();
    NX::Thread::join("power");
    NX::Thread::join("ipc");
    NX::Thread::join("gpio");
    NX::Thread::join("audio");

    // Finally delete service
    delete service;
    return 0;
}