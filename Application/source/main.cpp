#include "Application.hpp"
#include "Utils.hpp"
#include "Sysmodule.hpp"

int main(void) {

    // Start services
    romfsInit();
    socketInitializeDefault();

    #if _NXLINK_
        nxlinkStdio();
        Utils::writeStdout("=== stdout redirected to nxlink ===");
    #endif

    Sysmodule * sm = new Sysmodule();

    std::vector<std::string> aa = Utils::getFilesWithExt("/music/Bangas", ".mp3");
    for (size_t i = 0; i < aa.size(); i++) {
        Utils::writeStdout("==============================================");
        Utils::writeStdout("Parsing tags in " + aa[i] + "...");
        SongInfo si = Utils::getInfoFromID3(aa[i]);
        if (si.ID == -1) {
            Utils::writeStdout("Title: " + si.title + "\nArtist: " + si.artist + "\nAlbum: " + si.album);
        } else {
            Utils::writeStdout("No data in tags!");
        }
    }
    Utils::writeStdout("Found " + std::to_string(aa.size()) + " files!");

    // Start actual 'app' execution
    Main::Application * app = new Main::Application();
    app->run();
    delete app;

    delete sm;

    // Stop services
    romfsExit();
    socketExit();

    return 0;
}