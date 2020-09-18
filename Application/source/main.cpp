#include "Application.hpp"
#include "Log.hpp"
#include "utils/FS.hpp"

// Folder paths
#define MAIN_FOLDER "/switch/TriPlayer/"
#define ALBUM_IMAGES MAIN_FOLDER "images/album"
#define ARTIST_IMAGES MAIN_FOLDER "images/artist"
#define PLAYLIST_IMAGES MAIN_FOLDER "images/playlist"

// Log name
#define LOG_FILE "application.log"

int main(void) {
    // Ensure directories exist
    Utils::Fs::createPath(MAIN_FOLDER);
    Utils::Fs::createPath(ALBUM_IMAGES);
    Utils::Fs::createPath(ARTIST_IMAGES);
    Utils::Fs::createPath(PLAYLIST_IMAGES);

    // Start actual 'app' execution
    Main::Application * app = new Main::Application();
    app->run();
    delete app;

    return 0;
}