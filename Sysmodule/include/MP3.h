#ifndef MP3_H
#define MP3_H

// Essentially an MP3 'class' which is based on libmpg123

// Status of playback
enum MP3Status {
    Playing,    // Audio is being played
    Paused,     // Song is in middle of playback but is paused
    Stopped,    // No song is playing/paused
    Waiting     // Song finished decoding but waiting for buffers to finish
};

// Setup mpg123 (returns 0 on success)
int mp3Init();
// Close mpg123
void mp3Exit();

// Main loop which actually makes audio output!
void mp3Loop();

// 'Reset' state and play given song
void mp3Play(const char *);
// Resume playing current song
void mp3Resume();
// Pause current song
void mp3Pause();
// Stop playing the current song
void mp3Stop();

// Return status of playback
enum MP3Status mp3Status();
// Return percentage of song played (between 0 and 100)
double mp3Position();

// Returns the volume
double mp3Volume();
// Sets the volume
void mp3SetVolume(double);

#endif