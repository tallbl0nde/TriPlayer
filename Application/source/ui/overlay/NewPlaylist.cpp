#include "ui/overlay/NewPlaylist.hpp"

// Button dimensions
#define BUTTON_F 26
#define BUTTON_W 170
#define BUTTON_H 50

// Dimensions
#define PADDING 30
#define TEXTBOX_HEIGHT 50
#define WIDTH 650
#define HEIGHT 320

// Font sizes
#define HEADING_SIZE 36
#define SUBHEADING_SIZE 28

namespace CustomOvl {
    NewPlaylist::NewPlaylist() : Aether::Overlay() {
        // Close if B pressed
        this->onButtonPress(Aether::Button::B, [this]() {
            this->close();
        });

        // Add a second transparent layer cause we like it dark
        Aether::Rectangle * r = new Aether::Rectangle(this->x(), this->y(), this->w(), this->h());
        r->setColour(Aether::Colour{0, 0, 0, 120});
        this->addElement(r);

        // Background rectangle
        this->bg = new Aether::Rectangle((this->w() - WIDTH)/2, (this->h() - HEIGHT)/2, WIDTH, HEIGHT, 25);
        this->addElement(this->bg);

        // Heading
        this->heading = new Aether::Text(this->bg->x() + PADDING, this->bg->y() + PADDING, "|", HEADING_SIZE);
        this->addElement(this->heading);

        // Image
        this->image = new Aether::Image(this->heading->x(), this->heading->y() + this->heading->h() + 30, "romfs:/misc/noplaylist.png");
        int tmp = HEIGHT - 2*PADDING - this->heading->h() - 30;
        this->image->setWH(tmp, tmp);
        this->addElement(this->image);

        // Name heading
        this->nameHeading = new Aether::Text(this->image->x() + this->image->w() + PADDING, this->image->y(), "|", SUBHEADING_SIZE);
        this->addElement(this->nameHeading);

        // Name text box
        this->nameCallback = nullptr;
        this->name = new CustomElm::TextBox(this->nameHeading->x(), this->nameHeading->y() + this->nameHeading->h() + 10, this->bg->x() + this->bg->w() - this->nameHeading->x() - PADDING, TEXTBOX_HEIGHT);
        this->name->setKeyboardHint("Name");
        this->name->setKeyboardLimit(50);
        this->name->setKeyboardCallback([this]() {
            // Remove trailing spaces
            std::string str = this->name->string();
            while (!str.empty() && str[str.length() - 1] == ' ') {
                str.erase(str.length() - 1);
            }
            this->name->setString(str);

            // Call supplied callback
            if (this->nameCallback) {
                this->nameCallback(this->name->string());
            }
        });
        this->addElement(this->name);

        // Wrap buttons in a container
        Aether::Container * c = new Aether::Container(this->name->x(), this->name->y() + this->name->h() + 30, this->name->w(), BUTTON_H);
        this->addElement(c);

        // Cancel button
        this->cancel = new Aether::BorderButton(c->x() + this->name->w()/4 - BUTTON_W/2, c->y(), BUTTON_W, BUTTON_H, 2, "", BUTTON_F, [this]() {
            this->close();
        });
        c->addElement(this->cancel);

        // Save button
        this->ok = new Aether::FilledButton(c->x() + 3*this->name->w()/4 - BUTTON_W/2, c->y(), BUTTON_W, BUTTON_H, "", BUTTON_F, nullptr);
        this->ok->setTextColour(Aether::Colour{0, 0, 0, 255});
        c->addElement(this->ok);
        c->setFocused(this->ok);

        this->setFocused(this->name);
    }

    void NewPlaylist::setImage(Aether::Image * i) {
        i->setCallback(this->image->callback());
        i->setXY(this->image->x(), this->image->y());
        i->setWH(this->image->w(), this->image->h());
        this->removeElement(this->image);
        this->image = i;
        this->addElement(this->image);
    }

    void NewPlaylist::setImageCallback(std::function<void()> f) {
        this->image->setCallback(f);
    }

    void NewPlaylist::setNameCallback(std::function<void(std::string)> f) {
        this->nameCallback = f;
    }

    void NewPlaylist::setOKCallback(std::function<void()> f) {
        this->ok->setCallback(f);
    }

    void NewPlaylist::setCancelString(const std::string & s) {
        this->cancel->setString(s);
    }

    void NewPlaylist::setHeading(const std::string & s) {
        this->heading->setString(s);
    }

    void NewPlaylist::setNameString(const std::string & s) {
        this->nameHeading->setString(s);
    }

    void NewPlaylist::setOKString(const std::string & s) {
        this->ok->setString(s);
    }

    void NewPlaylist::setAccentColour(Aether::Colour c) {
        this->ok->setFillColour(c);
    }

    void NewPlaylist::setBackgroundColour(Aether::Colour c) {
        this->bg->setColour(c);
    }

    void NewPlaylist::setHeadingColour(Aether::Colour c) {
        this->heading->setColour(c);
        this->nameHeading->setColour(c);
    }

    void NewPlaylist::setTextBoxColour(Aether::Colour c) {
        this->name->setBoxColour(c);
    }

    void NewPlaylist::setTextColour(Aether::Colour c) {
        this->name->setTextColour(c);
    }
};