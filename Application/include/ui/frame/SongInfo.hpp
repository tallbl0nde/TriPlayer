#ifndef FRAME_SONGINFO_HPP
#define FRAME_SONGINFO_HPP

#include "ui/element/NumberBox.hpp"
#include "ui/element/TextBox.hpp"
#include "ui/frame/Frame.hpp"
#include "Types.hpp"

namespace Frame {
    class SongInfo : public Frame {
        private:
            // 'Cached' metadata which is updated before being saved/discarded
            Metadata::Song metadata;

            // Pointers to elements that get updated
            Aether::FilledButton * saveButton;
            CustomElm::TextBox * title;
            CustomElm::TextBox * artist;
            CustomElm::TextBox * album;
            CustomElm::NumberBox * discNumber;
            CustomElm::NumberBox * trackNumber;
            CustomElm::TextBox * filePath;

            // Functions which create/update popups
            Aether::MessageBox * msgbox;
            void createInfoOverlay(const std::string &);

            // Function which actually saves changes to database
            void saveChanges();

        public:
            // The constructor takes the ID of the song to show
            SongInfo(Main::Application *, SongID);

            // Update colours
            void updateColours();

            // Deletes any created popups
            ~SongInfo();
    };
};

#endif