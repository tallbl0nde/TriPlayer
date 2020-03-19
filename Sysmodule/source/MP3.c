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
    .num_voices      = 24,
    .num_effects     = 0,
    .num_sinks       = 1,
    .num_mix_objs    = 1,
    .num_mix_buffers = 2,
};

// Volume of mix
double volume = 1.0;
// Playback status
static enum MP3Status status;

/// === SONG DATA ===
// Number of channels in song
static int channels;
// Sample rate
static long rate;
// Samples read (so position in song)
static int readSamples = 0;
// Length of song in samples
static int totalSamples = 0;

// Wrappers for allocating/freeing buffers
// Allocate ~250kB per buffer
bool allocateBuffer() {
    bufferSize = ((250000) + 0xFFF) &~ 0xFFF;
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
    audrvUpdate(&audio);

    return true;
}

void freeBuffer() {
    if (decodedBuf[0] != NULL) {
        audrvMemPoolDetach(&audio, memPool[0]);
        audrvMemPoolRemove(&audio, memPool[0]);
        audrvUpdate(&audio);
        free(decodedBuf[0]);
        decodedBuf[0] = NULL;
    }
    if (decodedBuf[1] != NULL) {
        audrvMemPoolDetach(&audio, memPool[1]);
        audrvMemPoolRemove(&audio, memPool[1]);
        audrvUpdate(&audio);
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
    audrvUpdate(&audio);
    voiceAdded = 0;
}

// Deinitialize 'voice'
void voiceDrop() {
    if (voiceAdded != 0) {
        return;
    }

    audrvVoiceStop(&audio, voiceID);
    audrvUpdate(&audio);
    audrvVoiceDrop(&audio, voiceID);
    audrvUpdate(&audio);
}

// Decode the next frame(s) of the mp3 into buffer
// Returns bytes decoded (so zero if nothing!)
int decodeInto(int num) {
    memset(decodedBuf[num], 0, bufferSize);
    size_t decoded = 0;
    mpg123_read(mpg, decodedBuf[num], bufferSize, &decoded);
    readSamples += (decoded / (sizeof(s16) * channels));
    if (decoded == 0) {
        logMessage("[MP3] Error reading/decoding from file!");
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
    // Init mpg123
    int result = mpg123_init();
    if (result != MPG123_OK) {
        logMessage("[MP3] Error initializing mpg123");
        return -1;
    }

    // Set params + create instance
    mpg = mpg123_new(NULL, &result);
    if (mpg == NULL) {
        logMessage("[MP3] Error creating mpg123 instance");
        return -2;
    }
    result = mpg123_param(mpg, MPG123_FLAGS, MPG123_GAPLESS, 0.0);
    if (result != MPG123_OK) {
        logMessage("[MP3] Error enabling gapless decoding");
        return -3;
    }

    // Start audio service
    Result rc = audrenInitialize(&audConf);
    if (R_FAILED(rc)) {
        logMessage("[MP3] Error initializing audren");
        audrenOK = false;
        return -4;
    }
    audrenOK = true;

    // Create audio device
    rc = audrvCreate(&audio, &audConf, OUTPUT_CHANNELS);
    if (R_FAILED(rc)) {
        logMessage("[MP3] Error creating audio device");
        audrvOK = false;
        return -5;
    }
    audrvOK = true;

    // Allocate buffers
    if (!allocateBuffer()) {
        logMessage("[MP3] Unable to allocate buffers");
        return -6;
    }

    // Add output/sink
    sink = audrvDeviceSinkAdd(&audio, AUDREN_DEFAULT_DEVICE_NAME, OUTPUT_CHANNELS, sinkChannels);
    audrvUpdate(&audio);
    audrenStartAudioRenderer();

    logMessage("[MP3] mpg123 initialized successfully!");
    status = Stopped;
    return 0;
}

void mp3Exit() {
    mp3Stop();
    freeBuffer();
    mpg123_delete(mpg);
    if (audrenOK) {
        audrenExit();
    }
    if (audrvOK) {
        audrvClose(&audio);
    }
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
            // If not done pause for a bit
            svcSleepThread(1E+8);
        }

    } else {
        // If not playing block for 0.1 of a second
        svcSleepThread(1E+8);
    }
}

void mp3Play(const char * path) {
    // Get playback data
    int encoding;
    if (mpg123_open(mpg, path) == MPG123_OK) {
        if (mpg123_getformat(mpg, &rate, &channels, &encoding) != MPG123_OK) {
            logMessage("[MP3] Error getting format of mp3 file:");
            logMessage((char *) path);
            return;
        }

        // Clear previous song data
        readSamples = 0;
        totalSamples = mpg123_length(mpg);

        // Prepare 'voice'
        voiceInit();

        status = Playing;
    } else {
        logMessage("[MP3] Failed to open file:");
        logMessage((char *) path);
    }
}

void mp3Resume() {
    if (voiceAdded == 0) {
        audrvVoiceSetPaused(&audio, voiceID, false);
        audrvUpdate(&audio);
    }
    status = Playing;
}

void mp3Pause() {
    if (voiceAdded == 0) {
        audrvVoiceSetPaused(&audio, voiceID, true);
        audrvUpdate(&audio);
    }
    status = Paused;
}

void mp3Stop() {
    mpg123_close(mpg);
    voiceDrop();

    status = Stopped;
}

enum MP3Status mp3Status() {
    return status;
}

double mp3Position() {
    if (totalSamples == 0 || status == Stopped) {
        return 0.0;
    }

    return 100 * ((double)readSamples/totalSamples);
}

double mp3Volume() {
    return volume;
}

void mp3SetVolume(double d) {
    volume = d;
    audrvMixSetVolume(&audio, sink, (float) volume);
}