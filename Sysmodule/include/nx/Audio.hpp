#ifndef NX_AUDIO_HPP
#define NX_AUDIO_HPP

#include <atomic>
#include <cstddef>
#include <mutex>
#include "Types.hpp"

// Forward declare types
struct AudioDriverWaveBuf;

// The Audio class handles audio output, but not decoding.
// It is provided with decoded audio through public methods
// which can also be invoked to control various aspects
// of the output stream.
//
// It is a singleton class as we only ever want one instance
// shared across the entire service.
class Audio {
    public:
        // Status of audio playback
        enum class Status {
            Playing,                    // Buffers are being played
            Paused,                     // Buffers are queued but not playing
            Stopped                     // No buffers are queued/playing
        };

    private:
        // Constructor initializes audio output
        Audio();

        std::atomic<bool> exit_;        // Set true to stop looping
        static Audio * instance;        // Single instance of class
        std::mutex mutex;               // Mutex protecting all public methods
        std::atomic<bool> success;      // Indicates whether created successfullY

        int channels;                   // Channels in current song
        std::atomic<int> sampleOffset;  // Offset of voice's played sample count
        std::atomic<Status> status_;    // Current status of playback (see above enum)
        int voice;                      // ID of audio 'voice' (-1 if not set)
        std::atomic<double> vol;        // Current volume level (0.0 - 100.0)

        uint8_t ** memPool;             // Array of pointers to buffers containing decoded audio
        int nextBuf;                    // Index in waveBuf array of next buffer to fill
        int sink;                       // ID of audio 'sink'
        AudioDriverWaveBuf * waveBuf;   // Array of buffers

    public:
        // Delete copy constructors as this is a singleton
        Audio(Audio const &) = delete;
        void operator=(Audio const &) = delete;

        // Create or return instance
        static Audio * getInstance();

        // Returns true if the output device was initialized successfully
        bool initialized();
        // Call to indicate the main loop (process()) should stop and return
        void exit();

        // Append a buffer of audio data to play (does nothing if there is no free slot)
        // Takes pointer to buffer and it's size (does not free afterwards!)
        void addBuffer(uint8_t *, size_t);
        // Returns whether a buffer slot is available
        bool bufferAvailable();
        // Returns the maximum size of a single buffer
        size_t bufferSize();

        // Call to prepare the output device for a new song with the given info
        // Takes sample rate and number of channels
        void newSong(long, int);

        // Resume playback if paused
        void resume();
        // Pause playback if currently playing
        void pause();
        // Stop playback (discards buffers)
        void stop();
        // Return the current state of playback
        Status status();

        // Returns number of samples played
        int samplesPlayed();
        // Set the number of samples played so far (used when seeking)
        void setSamplesPlayed(int);

        // Return the current volume level (0.0 - 100.0)
        double volume();
        // Set the volume level (0.0 - 100.0)
        void setVolume(double);

        // Main function which continuously loops and plays buffers
        // Returns when exit() is called or an error occurs
        void process();

        // Delete the single audio object and clean up the audio device
        ~Audio();
};

#endif