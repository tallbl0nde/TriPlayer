#include "Log.h"
#include <malloc.h>
#include <mpg123.h>
#include "MP3.h"
#include <string.h>
#include <switch.h>

// Shoutout to KranKRival's sys-audioplayer, which I essentially
// referenced in order for this to actually work! ty <3

// Set zero if buffer is being played
static int bufPlaying[2];
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

// File to log to (thread-safe/won't be overwritten as these functions are only called in one thread)
FILE * logFile;

// Wrappers for allocating/freeing buffer
void allocateBuffer() {
    bufferSize = mpg123_outblock(mpg) * 4;
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
int decode(int num) {
    memset(decodedBuf[num], 0, bufferSize);
    size_t decoded = 0;
    mpg123_read(mpg, decodedBuf[num], bufferSize, &decoded);
    if (decoded == 0) {
        // Should probably have error handling here
        return 0;
    }

    // Fill audoutBuffer with decoded data
    audoutBuf[num].next = 0;
    audoutBuf[num].buffer = decodedBuf[num];
    audoutBuf[num].buffer_size = bufferSize;
    audoutBuf[num].data_size = bufferSize;
    audoutBuf[num].data_offset = 0;

    // Append to audout service
    audoutAppendAudioOutBuffer(&audoutBuf[num]);

    return (int)decoded;
}

int mp3Init() {
    logFile = NULL;

    // Init mpg123
    int status = mpg123_init();
    if (status != MPG123_OK) {
        logError(logFile, "[MP3] Error initializing mpg123", status);
        return -1;
    }

    // Set params + create instance
    mpg123_pars * params = mpg123_new_pars(&status);
    mpg123_par(params, MPG123_FORCE_RATE, audoutGetSampleRate(), 0);
    mpg123_par(params, MPG123_FORCE_STEREO, 1, 0);
    mpg = mpg123_parnew(params, NULL, &status);
    if (mpg == NULL) {
        logError(logFile, "[MP3] Error creating mpg123 instance", status);
        return -2;
    }

    // Allocate buffers
    allocateBuffer();
    bufPlaying[0] = 1;
    bufPlaying[1] = 1;

    status = Stopped;
    logSuccess(logFile, "[MP3] mpg123 initialized successfully!");
    return 0;
}

void mp3Exit() {
    logCloseFile(logFile);
    mp3Stop();
    freeBuffer();
    mpg123_delete(mpg);
    mpg123_exit();
}

void mp3Loop() {
    switch (status) {
        case Playing: {
            AudioOutBuffer * out;
            u32 outCount;

            // Enqueue another buffer
            if (bufPlaying[0] == 0) {
                audoutWaitPlayFinish(&out, &outCount, 3E+8);
                bufPlaying[0] = 1;
            }
            if (decode(0) == 0) {
                mp3Stop();
            } else {
                bufPlaying[0] = 0;
            }

            // Enqueue yet another buffer (to play while exiting loop/preparing buffer 0)
            if (bufPlaying[1] == 0) {
                audoutWaitPlayFinish(&out, &outCount, 3E+8);
                bufPlaying[1] = 1;
            }
            if (decode(1) == 0) {
                mp3Stop();
            } else {
                bufPlaying[1] = 0;
            }
        break;
        }

        case Paused: {
            AudioOutBuffer * out;
            u32 outCount;

            if (bufPlaying[0] == 0) {
                audoutWaitPlayFinish(&out, &outCount, 3E+8);
                bufPlaying[0] = 1;
            }
            if (bufPlaying[1] == 0) {
                audoutWaitPlayFinish(&out, &outCount, 3E+8);
                bufPlaying[1] = 1;
            }
        }

        case Stopped:
            // If not playing block for half a second
            svcSleepThread(5E+8);
            break;
    }
}

void mp3Play(const char * path) {
    // Get playback data
    int channels;
    int encoding;
    long rate;
    if (mpg123_open(mpg, path) == MPG123_OK) {
        if (mpg123_getformat(mpg, &rate, &channels, &encoding) != MPG123_OK) {
            logError(logFile, mpg123_plain_strerror(mpg123_errcode(mpg)), mpg123_errcode(mpg));
            return;
        }
        mpg123_format_none(mpg);
        mpg123_format(mpg, rate, channels, encoding);
        freeBuffer();
        allocateBuffer();

        status = Playing;
        logSuccess(logFile, "Playing song");
    } else {
        logError(logFile, "[MP3] Failed to open file", mpg123_errcode(mpg));
    }
}

void mp3Resume() {
    status = Playing;
}

void mp3Pause() {
    status = Paused;
}

void mp3Stop() {
    if (status == Playing) {
        mpg123_close(mpg);
    }
    status = Stopped;
}