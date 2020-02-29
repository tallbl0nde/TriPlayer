#include "Application.hpp"
#include "Utils.hpp"
#include "Sysmodule.hpp"

int main(void) {
    #if _NXLINK_
        socketInitializeDefault();
        nxlinkStdio();
        Utils::writeStdout("=== stdout redirected to nxlink ===");
    #endif

    // Start services
    romfsInit();

    Sysmodule * sm = new Sysmodule();

    std::vector<std::string> aa = Utils::getFilesWithExt("/music", ".mp3");
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

    #if _NXLINK_
        socketExit();
    #endif

    return 0;
}