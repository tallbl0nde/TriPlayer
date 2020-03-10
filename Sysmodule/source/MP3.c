#include "Log.h"
#include <malloc.h>
#include <mpg123.h>
#include "MP3.h"
#include <string.h>
#include <switch.h>

// Number of channels in output device
#define OUTPUT_CHANNELS 2
// Output ID
static int sink;
static const u8 sinkChannels[2] = {0, 1};

// Number of 'mpg123 outblocks' in each buffer
// Works out to ~250kB per buffer
#define OUTBLOCK_MULT 50
// WaveBuf structs
static AudioDriverWaveBuf waveBuf[2];
// Size of a buffer (not necessarily bytes decoded!)
static size_t bufferSize;
// Buffers for decoded data
static u8 * decodedBuf[2] = {NULL, NULL};
// MemPool IDs
static int memPool[2];
// Next buffer to fill/play
static int nextBuf;

// AudioDriver object
static AudioDriver audio;
// Voice ID
static const int voiceID = 0;
static int voiceAdded = 1;
// mpg123 instance
static mpg123_handle * mpg;

// Status of services
bool audrenOK;
bool audrvOK;

// These values work so I'm not touching them!
static const AudioRendererConfig audConf = {
    .output_rate     = AudioRendererOutputRate_48kHz,
    .num_voices      = 2,
    .num_effects     = 0,
    .num_sinks       = 1,
    .num_mix_objs    = 1,
    .num_mix_buffers = 2,
};

// Variables for currently playing song
static int channels;
static long rate;

// Status of playback
enum PlaybackStatus {
    Playing,
    Paused,
    Stopped
};
static enum PlaybackStatus status;

// File to log to (thread-safe/won't be overwritten as these functions are only called in one thread)
FILE * logFile;

// Wrappers for allocating/freeing buffers
bool allocateBuffer() {
    bufferSize = ((mpg123_outblock(mpg) * OUTBLOCK_MULT) + 0xFFF) &~ 0xFFF;
    for (size_t i = 0; i < 2; i++) {
        decodedBuf[i] = memalign(0x1000, bufferSize);
        memset(decodedBuf[i], 0, bufferSize);
        memset(&waveBuf[i], 0, sizeof(AudioDriverWaveBuf));
        waveBuf[i].state = AudioDriverWaveBufState_Done;
    }
    nextBuf = 0;

    if (decodedBuf[0] == NULL || decodedBuf[1] == NULL) {
        return false;
    }

    // Add buffers to memory pool
    memPool[0] = audrvMemPoolAdd(&audio, decodedBuf[0], bufferSize);
    audrvMemPoolAttach(&audio, memPool[0]);
    memPool[1] = audrvMemPoolAdd(&audio, decodedBuf[1], bufferSize);
    audrvMemPoolAttach(&audio, memPool[1]);

    return true;
}

void freeBuffer() {
    if (decodedBuf[0] != NULL) {
        audrvMemPoolDetach(&audio, memPool[0]);
        audrvMemPoolRemove(&audio, memPool[0]);
        free(decodedBuf[0]);
        decodedBuf[0] = NULL;
    }
    if (decodedBuf[1] != NULL) {
        audrvMemPoolDetach(&audio, memPool[1]);
        audrvMemPoolRemove(&audio, memPool[1]);
        free(decodedBuf[1]);
        decodedBuf[1] = NULL;
    }
}

// Initialize 'voice'
void voiceInit() {
    audrvVoiceInit(&audio, voiceID, channels, PcmFormat_Int16, rate);
    audrvVoiceSetDestinationMix(&audio, voiceID, AUDREN_FINAL_MIX_ID);
    if (channels == 1) {
        // Mono audio
        audrvVoiceSetMixFactor(&audio, voiceID, 1.0f, 0, 0);
        audrvVoiceSetMixFactor(&audio, voiceID, 1.0f, 0, 1);
    } else {
        // Stereo-o-o
        audrvVoiceSetMixFactor(&audio, voiceID, 1.0f, 0, 0);
        audrvVoiceSetMixFactor(&audio, voiceID, 0.0f, 0, 1);
        audrvVoiceSetMixFactor(&audio, voiceID, 0.0f, 1, 0);
        audrvVoiceSetMixFactor(&audio, voiceID, 1.0f, 1, 1);
    }
    audrvVoiceStart(&audio, voiceID);
    voiceAdded = 0;
}

// Deinitialize 'voice'
void voiceDrop() {
    if (voiceAdded != 0) {
        return;
    }

    audrvVoiceStop(&audio, voiceID);
    audrvVoiceDrop(&audio, voiceID);
}

// Decode the next frame(s) of the mp3 into buffer
// Returns bytes decoded (so zero if nothing!)
int decodeInto(int num) {
    memset(decodedBuf[num], 0, bufferSize);
    size_t decoded = 0;
    mpg123_read(mpg, decodedBuf[num], bufferSize, &decoded);
    if (decoded == 0) {
        logError(logFile, mpg123_plain_strerror(mpg123_errcode(mpg)), mpg123_errcode(mpg));
        return 0;
    }

    // Fill waveBuf with info
    waveBuf[num].data_raw = decodedBuf[num];
    waveBuf[num].size = decoded;
    waveBuf[num].start_sample_offset = 0;
    waveBuf[num].end_sample_offset = decoded/(2 * channels);

    return (int)decoded;
}

int mp3Init() {
    logFile = logOpenFile();

    // Init mpg123
    int status = mpg123_init();
    if (status != MPG123_OK) {
        logError(logFile, "[MP3] Error initializing mpg123", status);
        return -1;
    }

    // Set params + create instance
    mpg = mpg123_new(NULL, &status);
    if (mpg == NULL) {
        logError(logFile, "[MP3] Error creating mpg123 instance", status);
        return -2;
    }
    status = mpg123_param(mpg, MPG123_FLAGS, MPG123_GAPLESS, 0.0);
    if (status != MPG123_OK) {
        logError(logFile, "[MP3] Error enabling gapless decoding", status);
        return -3;
    }

    // Start audio service
    Result rc = audrenInitialize(&audConf);
    if (R_FAILED(rc)) {
        logError(logFile, "[MP3] Error initializing audren", rc);
        audrenOK = false;
        return -4;
    }
    audrenOK = true;

    // Create audio device
    rc = audrvCreate(&audio, &audConf, OUTPUT_CHANNELS);
    if (R_FAILED(rc)) {
        logError(logFile, "[MP3] Error creating audio device", rc);
        audrvOK = false;
        return -5;
    }
    audrvOK = true;

    // Add output/sink
    sink = audrvDeviceSinkAdd(&audio, AUDREN_DEFAULT_DEVICE_NAME, OUTPUT_CHANNELS, sinkChannels);
    audrvUpdate(&audio);
    audrenStartAudioRenderer();

    logSuccess(logFile, "[MP3] mpg123 initialized successfully!");
    status = Stopped;
    return 0;
}

void mp3Exit() {
    mp3Stop();
    mpg123_delete(mpg);
    if (audrenOK) {
        audrenExit();
    }
    if (audrvOK) {
        audrvClose(&audio);
    }
    logCloseFile(logFile);
}

void mp3Loop() {
    if (status == Playing) {
        audrvUpdate(&audio);

        // Only 'overwrite' buffer if it's finished playing
        if (waveBuf[nextBuf].state == AudioDriverWaveBufState_Done) {
            if (decodeInto(nextBuf) != 0) {
                audrvVoiceAddWaveBuf(&audio, voiceID, &waveBuf[nextBuf]);
            } else {
                status = Stopped;
            }
            nextBuf = ((nextBuf + 1) % 2);
        } else {
            svcSleepThread(1E+8);
        }

    } else {
        // If not playing block for 0.1 of a second
        logSuccess(logFile, "wrong loop");
        svcSleepThread(1E+8);
    }
}

void mp3Play(const char * path) {
    logSuccess(logFile, "[MP3] Called");
    // Get playback data
    int encoding;
    if (mpg123_open(mpg, path) == MPG123_OK) {
        logSuccess(logFile, "[MP3] Opened");
        if (mpg123_getformat(mpg, &rate, &channels, &encoding) != MPG123_OK) {
            logError(logFile, mpg123_plain_strerror(mpg123_errcode(mpg)), mpg123_errcode(mpg));
            return;
        }
        mpg123_format_none(mpg);
        mpg123_format(mpg, rate, channels, MPG123_ENC_SIGNED_16);

        // Allocate buffers
        if (!allocateBuffer()) {
            logError(logFile, "[MP3] Unable to allocate buffers", -1);
            return;
        }

        // Prepare 'voice'
        voiceInit();

        status = Playing;
        logSuccess(logFile, "[MP3] Playing song");
    } else {
        logError(logFile, "[MP3] Failed to open file", mpg123_errcode(mpg));
    }
}

void mp3Resume() {
    status = Playing;
}

void mp3Pause() {
    status = Paused;
    // ispaused thing
}

void mp3Stop() {
    if (status == Playing) {
        mpg123_close(mpg);
    }

    voiceDrop();
    freeBuffer();

    status = Stopped;
}