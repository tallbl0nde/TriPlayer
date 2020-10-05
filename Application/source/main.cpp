#include "Application.hpp"
#include "Paths.hpp"
#include "utils/FS.hpp"
#include "utils/NX.hpp"

int main(void) {
    // Ensure directories exist
    Utils::Fs::createPath(Path::Common::ConfigFolder);
    Utils::Fs::createPath(Path::Common::SwitchFolder);
    Utils::Fs::createPath(Path::App::AlbumImageFolder);
    Utils::Fs::createPath(Path::App::ArtistImageFolder);
    Utils::Fs::createPath(Path::App::PlaylistImageFolder);

    // Start switch related services early
    Utils::NX::startServices();

    // Start actual 'app' execution
    Main::Application * app = new Main::Application();
    app->run();
    delete app;

    Utils::NX::stopServices();

    return 0;
}

// #include <iostream>
// #include "ipc/TriPlayer.hpp"
// #include "Paths.hpp"
// #include "Log.hpp"
// #include <switch.h>

// int main(void) {
//     Log::openFile(Path::App::LogFile, Log::Level::Info);

//     // Temp vars
//     std::string tmpStr;
//     int tmpInt;
//     double tmpDbl;
//     size_t tmpSzt;
//     std::vector<int> tmpVec;
//     TriPlayer::Repeat tmpRep;
//     TriPlayer::Shuffle tmpShf;
//     TriPlayer::Status tmpSta;
//     std::vector<int> ids = {1, 2, 3, 4, 5};

//     // Init console
//     consoleInit(nullptr);

//     // Init service
//     std::cout << "Initializing... ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::initialize()) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;


//     // Version
//     std::cout << "Version: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::getVersion(tmpStr)) {
//         goto end;
//     }
//     std::cout << tmpStr << std::endl;


//     // Playback
//     std::cout << "Resume: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::resume()) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;

//     std::cout << "Pause: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::pause()) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;

//     std::cout << "Previous: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::previous()) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;

//     std::cout << "Next: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::next()) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;


//     // Volume
//     std::cout << "Setting volume to 50.5: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::setVolume(50.5)) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;

//     std::cout << "Get volume: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::getVolume(tmpDbl)) {
//         goto end;
//     }
//     std::cout << std::to_string(tmpDbl) << std::endl;

//     std::cout << "Mute: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::mute()) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;

//     std::cout << "Unmute: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::unmute(tmpDbl)) {
//         goto end;
//     }
//     std::cout << std::to_string(tmpDbl) << std::endl;


//     // Sub queue
//     std::cout << "Add song 1 to sub queue: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::addToSubQueue(1)) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;

//     std::cout << "Add song 15 to sub queue: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::addToSubQueue(15)) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;

//     std::cout << "Sub queue size: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::getSubQueueSize(tmpSzt)) {
//         goto end;
//     }
//     std::cout << std::to_string(tmpSzt) << std::endl;

//     std::cout << "Sub queue: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::getSubQueue(tmpVec)) {
//         goto end;
//     }
//     for (int i : tmpVec) {
//         std::cout << i << " ";
//     }
//     std::cout << std::endl;

//     std::cout << "Skip sub queue: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::skipSubQueueSongs(1)) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;

//     std::cout << "Remove song from sub queue: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::removeFromSubQueue(0)) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;


//     // Queue
//     std::cout << "Setting queue: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::setQueue(ids)) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;

//     std::cout << "Queue size: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::getQueueSize(tmpSzt)) {
//         goto end;
//     }
//     std::cout << std::to_string(tmpSzt) << std::endl;

//     std::cout << "Queue: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::getQueue(tmpVec)) {
//         goto end;
//     }
//     for (int i : tmpVec) {
//         std::cout << i << " ";
//     }
//     std::cout << std::endl;

//     std::cout << "Set pos to 2: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::setQueueIdx(2)) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;

//     std::cout << "Queue pos: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::getQueueIdx(tmpSzt)) {
//         goto end;
//     }
//     std::cout << std::to_string(tmpSzt) << std::endl;

//     std::cout << "Remove song: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::removeFromQueue(1)) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;


//     // Repeat
//     std::cout << "Set repeat to One: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::setRepeatMode(TriPlayer::Repeat::One)) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;

//     std::cout << "Get repeat mode: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::getRepeatMode(tmpRep)) {
//         goto end;
//     }
//     std::cout << (tmpRep == TriPlayer::Repeat::One ? "Pass" : "Fail") << std::endl;


//     // Shuffle
//     std::cout << "Set shuffle: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::setShuffleMode(TriPlayer::Shuffle::On)) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;

//     std::cout << "Get shuffle mode: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::getShuffleMode(tmpShf)) {
//         goto end;
//     }
//     std::cout << (tmpShf == TriPlayer::Shuffle::On ? "Pass" : "Fail") << std::endl;


//     // Info
//     std::cout << "Get song ID: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::getSongID(tmpInt)) {
//         goto end;
//     }
//     std::cout << std::to_string(tmpInt) << std::endl;

//     std::cout << "Get status: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::getStatus(tmpSta)) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;


//     // Position
//     std::cout << "Get position: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::getPosition(tmpDbl)) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;

//     std::cout << "Set position:";
//     consoleUpdate(NULL);
//     if (!TriPlayer::setPosition(42.0)) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;


//     // Text
//     std::cout << "Set text: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::setPlayingFromText("This is some text")) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;

//     std::cout << "Get text: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::getPlayingFromText(tmpStr)) {
//         goto end;
//     }
//     std::cout << tmpStr << std::endl;


//     // Database
//     std::cout << "Get lock: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::requestDatabaseLock()) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;

//     std::cout << "Release lock: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::releaseDatabaseLock()) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;


//     // Misc
//     std::cout << "Reload config: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::reloadConfig()) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;

//     std::cout << "Reset: ";
//     consoleUpdate(NULL);
//     if (!TriPlayer::reset()) {
//         goto end;
//     }
//     std::cout << "Done" << std::endl;
//     goto notend;

// end:
//     std::cout << "Failed" << std::endl;
//     consoleUpdate(NULL);

// notend:
//     // Wait for exit button
//     std::cout << "Press - to stop sysmodule" << std::endl;
//     std::cout << "Press + to exit" << std::endl;
//     consoleUpdate(NULL);
//     while (appletMainLoop()) {
//         hidScanInput();
//         u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

//         if (kDown & KEY_MINUS) {
//             TriPlayer::stopSysmodule();
//             break;
//         }

//         if (kDown & KEY_PLUS) {
//             break;
//         }

//         consoleUpdate(NULL);
//     }

//     // Clean up
//     Log::closeFile();
//     TriPlayer::exit();
//     consoleExit(nullptr);
//     return 0;
// }