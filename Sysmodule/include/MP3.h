#ifndef MP3_H
#define MP3_H

// Essentially an MP3 'class' which is based on libmpg123
// Must all be called within one thread! (except for those functions which are marked)

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

#endif