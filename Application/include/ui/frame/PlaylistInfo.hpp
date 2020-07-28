#ifndef FRAME_PLAYLISTINFO_HPP
#define FRAME_PLAYLISTINFO_HPP

#include <future>
#include "Types.hpp"
#include "ui/frame/Frame.hpp"

// Forward declarations for pointers
namespace CustomElm {
    class TextBox;
};

namespace CustomOvl {
    class FileBrowser;
};

namespace Frame {
    class PlaylistInfo : public Frame {
        private:
            // Type of file browser to create
            enum class FBType {
                Audio,      // Only permit audio (MP3) files
                Image       // Only permit image files
            };
            FBType fileType;

            // 'Cached' metadata which is updated before being saved/discarded
            Metadata::Playlist metadata;

            // Pointers to elements that get updated
            CustomElm::TextBox * name;
            CustomElm::TextBox * imagePath;
            Aether::Image * image;
            Aether::MessageBox * oldmsgbox;
            Aether::MessageBox * msgbox;
            CustomOvl::FileBrowser * browser;

            // Variables used for caching/saving images
            bool checkFB;
            std::vector<unsigned char> dlBuffer;
            bool updateImage;
            std::string newImagePath;

            // Functions which create/update popups
            void createFileBrowser(const FBType);
            void createInfoOverlay(const std::string &);

            // Functions to update frame based on type of image
            void removeImage();
            void updateImageFromID3(const std::string &);
            void updateImageFromPath(const std::string &);

            // Function which actually saves changes
            void saveChanges();

        public:
            // The constructor takes the ID of the playlist to show
            PlaylistInfo(Main::Application *, PlaylistID);

            // Checks for thread updates
            void update(uint32_t);

            // Deletes any created popups
            ~PlaylistInfo();
    };
};

#endif