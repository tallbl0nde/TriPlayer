#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <atomic>
#include <mutex>
#include <switch.h>
#include "Types.hpp"

// Config for audren
const AudioRendererConfig audrenCfg = {
    .output_rate     = AudioRendererOutputRate_48kHz,
    .num_voices      = 4,
    .num_effects     = 0,
    .num_sinks       = 1,
    .num_mix_objs    = 1,
    .num_mix_buffers = 2,
};

// Class that handles audio output (not decoding!)
// Essentially a wrapper for audrv
// Singleton as we only ever want one instance
class Audio {
    private:
        // = Class things =
        // Single instance
        static Audio * instance;
        // Initialize audio output
        Audio();
        // Mutex as this will be accessed across threads
        std::mutex mutex;

        // = Variables for current song =
        // Channel number
        int channels;
        // ID for voice (set to -1 if not created)
        int voice;

        // = Driver variables =
        // Buffer stuff
        AudioDriverWaveBuf * waveBuf;
        int nextBuf;
        // Driver object
        AudioDriver drv;
        // True when object should stop looping
        std::atomic<bool> exit_;
        // Pointer to memory pools (decoded data)
        u8 ** memPool;
        // ID for sink
        int sink;
        // Bool indicating if initialized/created successfully
        std::atomic<bool> success;

        // Offset for samples played by voice
        int sampleOffset;
        // Playback status
        std::atomic<AudioStatus> status_;
        // Volume (0 - 100.0)
        std::atomic<double> vol;

    public:
        Audio(Audio const &)           = delete;
        void operator=(Audio const &)  = delete;

        // Create or return instance
        static Audio * getInstance();

        // Returns true if initialized correctly
        bool initialized();
        // Call to indicate to stop looping
        void exit();

        // Append an audio buffer to play (does nothing if no room!)
        // Pointer to buffer and number of bytes
        // Does not free the provided buffer!
        void addBuffer(u8 *, size_t);
        // Returns true if a buffer is available
        bool bufferAvailable();
        // Returns size of a buffer (max bytes to decode)
        size_t bufferSize();

        // Call to prepare for new song (sets up audio regarding rate and mono/stereo)
        // Takes sample rate and number of channels
        void newSong(long, int);
        void resume();              // Resume if paused
        void pause();               // Pause if playing
        void stop();                // Stop playback (discards buffers)
        AudioStatus status();       // Return state of playback

        // Returns number of samples played
        int samplesPlayed();
        // Set the number of samples played (used when seeking)
        void setSamplesPlayed(int);

        // Volume (0 - 100.0)
        double volume();
        void setVolume(double);

        // Main function which should be run in it's own thread
        // Continuously loops and plays buffers, etc... until exit() called
        void process();

        // Cleanup
        ~Audio();
};

#endif