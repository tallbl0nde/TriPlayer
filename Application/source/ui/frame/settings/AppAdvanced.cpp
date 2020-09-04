#include "Application.hpp"
#include <filesystem>
#include "ui/frame/settings/AppAdvanced.hpp"
#include "ui/overlay/ProgressBox.hpp"
#include "utils/FS.hpp"
#include "utils/metadata/Metadata.hpp"

// Image locations
#define ARTIST_IMAGE_LOCATION "/switch/TriPlayer/images/artist/"
#define ALBUM_IMAGE_LOCATION "/switch/TriPlayer/images/album/"

namespace Frame::Settings {
    AppAdvanced::AppAdvanced(Main::Application * a) : Frame(a) {
        // Temporary variables
        Config * cfg = this->app->config();

        // Advanced::auto_launch_service
        this->addToggle("Auto Launch Sysmodule", [cfg]() -> bool {
            return cfg->autoLaunchService();
        }, [cfg](bool b) {
            cfg->setAutoLaunchService(b);
        });
        this->addComment("Automatically attempt to start the sysmodule if it is not running when the app is launched.");
        this->list->addElement(new Aether::ListSeparator());

        // Scan now
        this->addButton("Scan Now", [this]() {
            // fix this
            // this->app->popScreen();
            // this->app->setScreen(Main::ScreenID::Splash);
        });
        this->list->addElement(new Aether::ListSeparator());

        // Remove redundant images
        this->addButton("Remove Unneeded Images", [this]() {
            this->removeImages();
        });
        this->addComment("Remove any images within '/switch/TriPlayer/images' that are no longer needed. TriPlayer aims to keep this folder clean and up-to-date so this shouldn't remove any files under normal circumstances.");

        // Search for images
        this->addButton("Search for Missing Album Images", [this]() {
            this->getAlbumImages();
        });
        this->addButton("Search for Missing Artist Images", [this]() {
            this->getArtistImages();
        });
        this->addComment("Scrape TheAudioDB for images that are missing from your library. Depending on the size of your library, these could take a while. These processes cannot be cancelled once started.");

        // Create overlay
        this->ovlSearch = new CustomOvl::ProgressBox();
        this->ovlSearch->setBackgroundColour(this->app->theme()->popupBG());
        this->ovlSearch->setTextColour(this->app->theme()->FG());
        this->ovlSearch->setMutedTextColour(this->app->theme()->muted());
        this->ovlSearch->setBarBackgroundColour(this->app->theme()->muted2());
        this->ovlSearch->setBarForegroundColour(this->app->theme()->accent());
        this->searchRunning = false;
    }

    void AppAdvanced::getAlbumImages() {
        // Create overlay
        this->ovlSearch->setHeadingText("Searching for Album Images...");
        this->ovlSearch->setSubheadingText("(0 out of 0)");
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

    void AppAdvanced::getArtistImages() {
        // Create overlay
        this->ovlSearch->setHeadingText("Searching for Artist Images...");
        this->ovlSearch->setSubheadingText("(0 out of 0)");
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

    void AppAdvanced::removeImages() {
        // Get list of all images referenced in database (returned in order)
        bool ok;
        std::vector<std::string> dbFiles = this->app->database()->getAllImagePaths(ok);
        if (!ok) {
            return;
        }

        // Get list of all files in folder
        std::vector<std::string> folderFiles;
        for (auto & entry: std::filesystem::recursive_directory_iterator("/switch/TriPlayer/images/")) {
            folderFiles.push_back(entry.path());
        }
        std::sort(folderFiles.begin(), folderFiles.end());

        // Remove from folder where not in DB
        for (size_t i = 0; i < folderFiles.size(); i++) {
            bool inDB = std::binary_search(dbFiles.begin(), dbFiles.end(), folderFiles[i]);
            if (!inDB) {
                Utils::Fs::deleteFile(folderFiles[i]);
            }
        }
    }

    void AppAdvanced::searchAlbumsThread() {
        // Get list of albums to search for
        std::vector<Metadata::Album> albums = this->app->database()->getAllAlbumMetadata();
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
                // If successful, write to file
                std::string filename = ALBUM_IMAGE_LOCATION + std::to_string(id) + ".png";
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

    void AppAdvanced::searchArtistsThread() {
        // Get list of artists to search for
        std::vector<Metadata::Artist> artists = this->app->database()->getAllArtistMetadata();
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
                // If successful, write to file
                std::string filename = ARTIST_IMAGE_LOCATION + std::to_string(id) + ".png";
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

    void AppAdvanced::update(uint32_t dt) {
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
                    std::string str = this->searchName + " (" + std::to_string(this->searchCurrent) + " out of " + std::to_string(this->searchMax) + ")";
                    mtx.unlock();
                    this->ovlSearch->setSubheadingText(str);
                    this->ovlSearch->setValue(100.0 * per);
                }
            }
        }
    }

    AppAdvanced::~AppAdvanced() {
        delete this->ovlSearch;
    }
};