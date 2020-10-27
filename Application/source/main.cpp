#include "Application.hpp"
#include "Paths.hpp"
#include "utils/FS.hpp"
#include "utils/NX.hpp"

int main(void) {
    // Ensure directories exist
    Utils::Fs::createPath(Path::Common::ConfigFolder);
    Utils::Fs::createPath(Path::Common::SwitchFolder);
    Utils::Fs::createPath(Path::App::UpdateFolder);
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