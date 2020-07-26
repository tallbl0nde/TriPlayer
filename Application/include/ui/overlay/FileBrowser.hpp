#ifndef OVERLAY_FILEBROWSER_HPP
#define OVERLAY_FILEBROWSER_HPP

#include "ui/overlay/Overlay.hpp"

namespace CustomOvl {
    // The FileBrowser overlay presents a basic file explorer which is
    // used to select a file on the filesystem (who woulda guessed?)
    class FileBrowser : public Overlay {
        private:
            // Elements
            Aether::Rectangle * topR;
            Aether::Rectangle * bottomR;
            Aether::Rectangle * rect;
            Aether::BorderButton * cancelButton;
            Aether::List * list;
            Aether::Text * heading;
            Aether::Text * path;

            // Path to chosen file
            std::string file;
            // Allowed extensions
            std::vector<std::string> exts;

            // Colours
            Aether::Colour text;

            // Fill list with contents of directory
            void populateList();
            // Set chosen file
            void setFile(const std::string &);

        public:
            // Constructor takes width, height (automatically centred) and path
            FileBrowser(int, int);

            // Returns path to selected file (empty if cancelled)
            std::string chosenFile();
            // Reset the chosen file
            void resetFile();

            // Set allowed extensions (call before setting path!)
            // Include . in extension (eg. '.mp3')
            void setExtensions(const std::vector<std::string>);
            // Set path
            void setPath(const std::string &);

            // Set text
            void setCancelText(std::string);
            void setHeadingText(std::string);

            // Set colours
            void setAccentColour(Aether::Colour);
            void setMutedLineColour(Aether::Colour);
            void setMutedTextColour(Aether::Colour);
            void setRectangleColour(Aether::Colour);
            void setTextColour(Aether::Colour);

            // Catch B events to move up the tree (or close)
            bool handleEvent(Aether::InputEvent *);
    };
};

#endif