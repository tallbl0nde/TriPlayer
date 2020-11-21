#ifndef FRAME_ARTISTINFO_HPP
#define FRAME_ARTISTINFO_HPP

#include <future>
#include "ui/element/TextBox.hpp"
#include "ui/frame/Frame.hpp"
#include "ui/overlay/FileBrowser.hpp"
#include "meta/Metadata.hpp"

namespace Frame {
    class ArtistInfo : public Frame {
        private:
            // 'Cached' metadata which is updated before being saved/discarded
            Metadata::Artist metadata;

            // Pointers to elements that get updated
            Aether::FilledButton * saveButton;
            CustomElm::TextBox * name;
            CustomElm::TextBox * imagePath;
            Aether::Image * image;
            Aether::MessageBox * oldmsgbox;
            Aether::MessageBox * msgbox;
            CustomOvl::FileBrowser * browser;

            // Variables used for caching/saving images
            bool checkFB;
            std::vector<unsigned char> dlBuffer;
            int dlID;
            bool updateImage;
            std::future<Metadata::DownloadResult> dlThread;
            bool threadRunning;
            std::string newImagePath;

            // Functions which create/update popups
            void createAudioDBOverlay();
            void updateAudioDBOverlay();
            void createFileBrowser();
            void createInfoOverlay(const std::string &);

            // Functions to update frame based on type of image
            void removeImage();
            void updateImageFromDL();
            void updateImageFromPath(const std::string &);

            // Function which actually saves changes
            void saveChanges();

        public:
            // The constructor takes the ID of the artist to show
            ArtistInfo(Main::Application *, ArtistID);

            // Update colours
            void updateColours();

            // Checks for thread updates
            void update(uint32_t);

            // Deletes any created popups
            ~ArtistInfo();
    };
};

#endif