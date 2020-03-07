#include "Log.h"
#include <malloc.h>
#include <mpg123.h>
#include "MP3.h"
#include <string.h>
#include <switch.h>

// Shoutout to KranKRival's sys-audioplayer, which I essentially
// referenced in order for this to actually work! ty <3

// Index of 'active' buffer
static uint8_t activeBuf;
// Buffers for AudioOut
static AudioOutBuffer audoutBuf[2];
// Buffers for decoded data
static u8 * decodedBuf[2];
// Size of a buffer
static size_t bufferSize;
// mpg123 instance
static mpg123_handle * mpg;
// Status of playback
enum PlaybackStatus {
    Playing,
    Paused,
    Stopped
};
static enum PlaybackStatus status;

// Wrappers for allocating/freeing buffer
void allocateBuffer() {
    bufferSize = mpg123_outblock(mpg) * 16;
    decodedBuf[0] = memalign(0x1000, bufferSize);
    decodedBuf[1] = memalign(0x1000, bufferSize);
}

void freeBuffer() {
    if (decodedBuf[0] != NULL) {
        free(decodedBuf[0]);
        decodedBuf[0] = NULL;
    }
    if (decodedBuf[1] != NULL) {
        free(decodedBuf[1]);
        decodedBuf[1] = NULL;
    }
}

// Decode the next frame(s) of the mp3 into buffer
// Returns bytes decoded (so zero if nothing!)
int decode() {
    memset(decodedBuf[activeBuf], 0, bufferSize);
    size_t decoded = 0;
    mpg123_read(mpg, decodedBuf[activeBuf], bufferSize, &decoded);
    if (decoded == 0) {
        // Should probably have error handling here
        return 0;
    }

    // Fill audoutBuffer with decoded data
    audoutBuf[activeBuf].next = 0;
    audoutBuf[activeBuf].buffer = decodedBuf[activeBuf];
    audoutBuf[activeBuf].buffer_size = bufferSize;
    audoutBuf[activeBuf].data_size = bufferSize;
    audoutBuf[activeBuf].data_offset = 0;

    // Append to audout service
    audoutAppendAudioOutBuffer(&audoutBuf[activeBuf]);

    // Indicate other buffer should be used next
    activeBuf = (activeBuf == 0 ? 1 : 0);
    return (int)decoded;
}

int mp3Init() {
    // Init mpg123
    int status = mpg123_init();
    if (status != MPG123_OK) {
        logError("[MP3] Error initializing mpg123", status);
        return -1;
    }

    // Set params + create instance
    mpg123_pars * params = mpg123_new_pars(&status);
    mpg123_par(params, MPG123_FORCE_RATE, audoutGetSampleRate(), 0);
    mpg123_par(params, MPG123_FORCE_STEREO, 1, 0);
    mpg = mpg123_parnew(params, NULL, &status);
    if (mpg == NULL) {
        logError("[MP3] Error creating mpg123 instance", status);
        return -2;
    }

    // Allocate buffers
    allocateBuffer();
    activeBuf = 0;

    status = Stopped;
    logSuccess("[MP3] mpg123 initialized successfully!");
    return 0;
}

void mp3Exit() {
    freeBuffer();
    mpg123_close(mpg);
    mpg123_delete(mpg);
    mpg123_exit();
}

void mp3Loop() {
    if (status == Playing) {
        // Set to zero when finished playing
        int done = 1;

        // Decode next buffers
        done = decode();
        if (done != 0) {
            done = decode();
        }

        if (done != 0) {
            // Wait for buffers to finish playing
            AudioOutBuffer * out;
            u32 outCount;
            audoutWaitPlayFinish(&out, &outCount, 1E+9);
            audoutWaitPlayFinish(&out, &outCount, 1E+9);
        } else {
            status = Stopped;
        }
    } else {
        svcSleepThread(5E+9);
    }
}

void mp3Play(const char * path) {
    // Get playback data
    int channels;
    int encoding;
    long rate;
    if (mpg123_open(mpg, path) == MPG123_OK) {
        if (mpg123_getformat(mpg, &rate, &channels, &encoding) != MPG123_OK) {
            logError(mpg123_plain_strerror(mpg123_errcode(mpg)), mpg123_errcode(mpg));
            return;
        }
        mpg123_format_none(mpg);
        mpg123_format(mpg, rate, channels, encoding);
        freeBuffer();
        allocateBuffer();

        status = Playing;
        logSuccess("Playing song");
    } else {
        logError("[MP3] Failed to open file", mpg123_errcode(mpg));
    }
}

void mp3Resume() {
    status = Playing;
}

void mp3Pause() {
    status = Paused;
}