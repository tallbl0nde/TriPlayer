#ifndef FRAME_ARTISTINFO_HPP
#define FRAME_ARTISTINFO_HPP

#include <future>
#include "ui/element/TextBox.hpp"
#include "ui/frame/Frame.hpp"
#include "utils/metadata/Metadata.hpp"
#include "Types.hpp"

namespace Frame {
    class ArtistInfo : public Frame {
        private:
            // 'Cached' metadata which is updated before being saved/discarded
            Metadata::Artist metadata;

            // Pointers to elements that get updated
            CustomElm::TextBox * imagePath;
            Aether::Image * image;
            Aether::MessageBox * oldmsgbox;
            Aether::MessageBox * msgbox;

            // Buffer to fill with image
            std::vector<unsigned char> dlBuffer;
            // ID of downloaded image
            int dlID;
            // Future for running download thread
            std::future<Metadata::DownloadResult> dlThread;
            // Set true if the thread is started
            bool threadRunning;

            // Functions which create/update popups
            void createAudioDBOverlay();
            void updateAudioDBOverlay1();
            void updateAudioDBOverlay2(const std::string &);

        public:
            // The constructor takes the ID of the artist to show
            ArtistInfo(Main::Application *, ArtistID);

            // Checks for thread updates
            void update(uint32_t);

            // Deletes any created popups
            ~ArtistInfo();
    };
};

#endif