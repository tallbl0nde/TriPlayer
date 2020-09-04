#ifndef FRAME_SETTINGS_APPADVANCED
#define FRAME_SETTINGS_APPADVANCED

#include "ui/frame/settings/Frame.hpp"
#include "ui/overlay/ProgressBox.hpp"

namespace Frame::Settings {
    // This frame contains 'advanced' settings (who woulda guessed?)
    class AppAdvanced : public Frame {
        private:
            // 'Progress' overlay
            CustomOvl::ProgressBox * ovlSearch;

            // Helpers called to scrape a provider for images
            void getAlbumImages();
            void getArtistImages();

            // Helper called by 'Remove Images' to clean up database and folder
            void removeImages();

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
            // Constructor creates needed elements
            AppAdvanced(Main::Application *);

            // Checks for thread updates
            void update(uint32_t);

            // Delete overlay
            ~AppAdvanced();
    };
};

#endif