#ifndef FRAME_SETTINGS_APPMETADATA
#define FRAME_SETTINGS_APPMETADATA

#include "ui/frame/settings/Frame.hpp"
#include "ui/overlay/ProgressBox.hpp"

namespace Frame::Settings {
    // Contains metadata related settings
    class AppMetadata : public Frame {
        private:
            // 'Progress' overlay
            CustomOvl::ProgressBox * ovlSearch;

            // Helpers called to scrape a provider for images
            void getAlbumImages();
            void getArtistImages();

            // === Thread things === //
            // Future to run search on
            std::future<void> searchThread;

            // Variables shared between threads
            std::mutex searchMtx;
            std::string searchName;
            size_t searchCurrent;
            size_t searchLast;
            size_t searchMax;
            bool searchRunning;

            // Functions that actually search
            void searchAlbumsThread();
            void searchArtistsThread();
        public:
            // Constructor sets up elements
            AppMetadata(Main::Application *);

            // Checks for thread updates
            void update(uint32_t);

            // Delete overlay
            ~AppMetadata();
    };
};

#endif