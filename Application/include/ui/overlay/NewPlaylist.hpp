#ifndef OVERLAY_NEWPLAYLIST_HPP
#define OVERLAY_NEWPLAYLIST_HPP

#include "ui/element/TextBox.hpp"

namespace CustomOvl {
    // Overlay shown when creating a new playlist
    class NewPlaylist : public Aether::Overlay {
        private:
            // Elements
            Aether::Rectangle * bg;
            Aether::Text * heading;
            Aether::Image * image;
            Aether::Text * nameHeading;
            CustomElm::TextBox * name;
            Aether::FilledButton * ok;
            Aether::BorderButton * cancel;

            // Callback for text box
            std::function<void(std::string)> nameCallback;

        public:
            // Constructor prepares elements
            NewPlaylist();

            // Image related things
            void setImage(Aether::Image *);

            // Callbacks
            void setImageCallback(std::function<void()>);
            void setNameCallback(std::function<void(std::string)>);
            void setOKCallback(std::function<void()>);

            // Set strings
            void setCancelString(const std::string &);
            void setHeading(const std::string &);
            void setNameString(const std::string &);
            void setOKString(const std::string &);

            // Set colours
            void setAccentColour(Aether::Colour);
            void setBackgroundColour(Aether::Colour);
            void setHeadingColour(Aether::Colour);
            void setTextBoxColour(Aether::Colour);
            void setTextColour(Aether::Colour);
    };
};

#endif