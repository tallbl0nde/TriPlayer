#ifndef FRAME_ALBUMINFO_HPP
#define FRAME_ALBUMINFO_HPP

#include <future>
#include "ui/element/TextBox.hpp"
#include "ui/frame/Frame.hpp"
#include "ui/overlay/FileBrowser.hpp"
#include "meta/Metadata.hpp"

namespace Frame {
    class AlbumInfo : public Frame {
        private:
            // Type of file browser to create
            enum class FBType {
                Audio,      // Only permit audio (MP3) files
                Image       // Only permit image files
            };
            FBType fileType;

            // 'Cached' metadata which is updated before being saved/discarded
            Metadata::Album metadata;

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
            void createFileBrowser(const FBType);
            void createInfoOverlay(const std::string &);

            // Functions to update frame based on type of image
            void removeImage();
            void updateImageFromDL();
            void updateImageFromTag(const std::string &);
            void updateImageFromPath(const std::string &);

            // Function which actually saves changes
            void saveChanges();

        public:
            // The constructor takes the ID of the artist to show
            AlbumInfo(Main::Application *, AlbumID);

            // Update colours
            void updateColours();

            // Checks for thread updates
            void update(uint32_t);

            // Deletes any created popups
            ~AlbumInfo();
    };
};

#endif