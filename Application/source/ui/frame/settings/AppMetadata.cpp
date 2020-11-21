#include "Application.hpp"
#include "lang/Lang.hpp"
#include "Paths.hpp"
#include "ui/frame/settings/AppMetadata.hpp"
#include "ui/overlay/ProgressBox.hpp"
#include "utils/FS.hpp"
#include "utils/Image.hpp"
#include "utils/metadata/Metadata.hpp"
#include "utils/Utils.hpp"

namespace Frame::Settings {
    AppMetadata::AppMetadata(Main::Application * a) : Frame(a) {
        // Temporary variables
        Config * cfg = this->app->config();

        // Metadata::scan_on_launch
        this->addToggle("Settings.AppMetadata.ScanOnLaunch"_lang, [cfg]() -> bool {
            return cfg->scanOnLaunch();
        }, [cfg](bool b) {
            cfg->setScanOnLaunch(b);
        });
        this->addComment("Settings.AppMetadata.ScanOnLaunchText"_lang);

        // Scan now
        this->addButton("Settings.AppMetadata.ScanNow"_lang, [this]() {
            this->app->popScreen();
            this->app->setScreen(Main::ScreenID::Splash);
        });
        this->addComment("Settings.AppMetadata.ScanNowText"_lang);
        this->list->addElement(new Aether::ListSeparator());

        // Search for images
        this->addButton("Settings.AppMetadata.SearchAlbumImages"_lang, [this]() {
            this->getAlbumImages();
        });
        this->addButton("Settings.AppMetadata.SearchArtistImages"_lang, [this]() {
            this->getArtistImages();
        });
        this->addComment("Settings.AppMetadata.SearchImagesText"_lang);

        // Create overlay
        this->ovlSearch = new CustomOvl::ProgressBox();
        this->ovlSearch->setBackgroundColour(this->app->theme()->popupBG());
        this->ovlSearch->setTextColour(this->app->theme()->FG());
        this->ovlSearch->setMutedTextColour(this->app->theme()->muted());
        this->ovlSearch->setBarBackgroundColour(this->app->theme()->muted2());
        this->ovlSearch->setBarForegroundColour(this->app->theme()->accent());
        this->searchRunning = false;
    }

    void AppMetadata::getAlbumImages() {
        // Create overlay
        this->ovlSearch->setHeadingText("Settings.AppMetadata.SearchAlbumImagesText"_lang);
        this->ovlSearch->setSubheadingText(Utils::substituteTokens("Settings.AppMetadata.DownloadProgress"_lang, "", "0", "0"));
        this->ovlSearch->setValue(0.0);
        this->app->addOverlay(this->ovlSearch);

        // Initialize vars
        this->searchCurrent = 0;
        this->searchLast = 0;
        this->searchMax = 0;
        this->searchName = "";

        // Start thread
        this->searchThread = std::async(std::launch::async, [this]() {
            this->searchAlbumsThread();
        });
        this->searchRunning = true;
    }

    void AppMetadata::getArtistImages() {
        // Create overlay
        this->ovlSearch->setHeadingText("Settings.AppMetadata.SearchArtistImagesText"_lang);
        this->ovlSearch->setSubheadingText(Utils::substituteTokens("Settings.AppMetadata.DownloadProgress"_lang, "", "0", "0"));
        this->ovlSearch->setValue(0.0);
        this->app->addOverlay(this->ovlSearch);

        // Initialize vars
        this->searchCurrent = 0;
        this->searchLast = 0;
        this->searchMax = 0;
        this->searchName = "";

        // Start thread
        this->searchThread = std::async(std::launch::async, [this]() {
            this->searchArtistsThread();
        });
        this->searchRunning = true;
    }

    void AppMetadata::searchAlbumsThread() {
        // Get list of albums to search for
        std::vector<Metadata::Album> albums = this->app->database()->getAllAlbumMetadata(Database::SortBy::AlbumAsc);
        albums.erase(std::remove_if(albums.begin(), albums.end(), [](const Metadata::Album m) {
            return !m.imagePath.empty();
        }), albums.end());

        // Lock database for writing
        this->app->lockDatabase();

        // Iterate over each artist
        std::vector<unsigned char> buffer;
        int id;
        for (size_t i = 0; i < albums.size(); i++) {
            // Update shared variables
            std::unique_lock<std::mutex> mtx(this->searchMtx);
            this->searchName = albums[i].name;
            this->searchCurrent = i + 1;
            this->searchMax = albums.size();
            mtx.unlock();

            // Search for and download image using album name (skips over any errors)
            buffer.clear();
            if (Metadata::downloadAlbumImage(albums[i].name, buffer, id) == Metadata::DownloadResult::Success) {
                // If successful, resize and write to file
                bool resized = Utils::Image::resize(buffer, 400, 400);
                if (!resized) {
                    Log::writeWarning("[META] Couldn't resize album image, saving with original dimensions");
                }

                std::string filename = Path::App::AlbumImageFolder + std::to_string(id) + ".png";
                if (!Utils::Fs::writeFile(filename, buffer)) {
                    continue;
                }

                // Update database, deleting file if an error occurs
                albums[i].tadbID = id;
                albums[i].imagePath = filename;
                if (!this->app->database()->updateAlbum(albums[i])) {
                    Utils::Fs::deleteFile(filename);
                }
            }
        }

        // Unlock database
        this->app->unlockDatabase();
    }

    void AppMetadata::searchArtistsThread() {
        // Get list of artists to search for
        std::vector<Metadata::Artist> artists = this->app->database()->getAllArtistMetadata(Database::SortBy::ArtistAsc);
        artists.erase(std::remove_if(artists.begin(), artists.end(), [](const Metadata::Artist m) {
            return !m.imagePath.empty();
        }), artists.end());

        // Lock database for writing
        this->app->lockDatabase();

        // Iterate over each artist
        std::vector<unsigned char> buffer;
        int id;
        for (size_t i = 0; i < artists.size(); i++) {
            // Update shared variables
            std::unique_lock<std::mutex> mtx(this->searchMtx);
            this->searchName = artists[i].name;
            this->searchCurrent = i + 1;
            this->searchMax = artists.size();
            mtx.unlock();

            // Search for and download image using artist name (skips over any errors)
            buffer.clear();
            if (Metadata::downloadArtistImage(artists[i].name, buffer, id) == Metadata::DownloadResult::Success) {
                // If successful, resize and write to file
                bool resized = Utils::Image::resize(buffer, 400, 400);
                if (!resized) {
                    Log::writeWarning("[META] Couldn't resize artist image, saving with original dimensions");
                }

                std::string filename = Path::App::ArtistImageFolder + std::to_string(id) + ".png";
                if (!Utils::Fs::writeFile(filename, buffer)) {
                    continue;
                }

                // Update database, deleting file if an error occurs
                artists[i].tadbID = id;
                artists[i].imagePath = filename;
                if (!this->app->database()->updateArtist(artists[i])) {
                    Utils::Fs::deleteFile(filename);
                }
            }
        }

        // Unlock database
        this->app->unlockDatabase();
    }

    void AppMetadata::update(uint32_t dt) {
        Frame::update(dt);

        // Check if thread is finished
        if (this->searchRunning) {
            // Close the overlay once it's done
            if (this->searchThread.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                this->ovlSearch->close();
                this->searchRunning = false;

            // Otherwise update overlay values
            } else {
                std::unique_lock<std::mutex> mtx(this->searchMtx);
                if (this->searchLast != this->searchCurrent) {
                    float per = this->searchCurrent/(float)this->searchMax;
                    std::string str = Utils::substituteTokens("Settings.AppMetadata.DownloadProgress"_lang, this->searchName, std::to_string(this->searchCurrent), std::to_string(this->searchMax));
                    mtx.unlock();
                    this->ovlSearch->setSubheadingText(str);
                    this->ovlSearch->setValue(100.0 * per);
                }
            }
        }
    }

    AppMetadata::~AppMetadata() {
        delete this->ovlSearch;
    }
};