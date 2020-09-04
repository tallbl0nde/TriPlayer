#include "ui/overlay/ProgressBox.hpp"
#include "utils/Utils.hpp"

// Appearance parameters
#define HEIGHT 210
#define WIDTH 600
#define CORNER_RADIUS 14
#define HEADING_FONT_SIZE 26
#define SUBHEADING_FONT_SIZE 22
#define X_PADDING 50
#define Y_PADDING 40

namespace CustomOvl {
    ProgressBox::ProgressBox() : Overlay() {
        // Don't allow closing by tap
        this->setTopLeft(0, 0);
        this->setBottomRight(1280, 720);

        // Create elements
        this->bg = new Aether::Rectangle(this->x() + (this->w() - WIDTH)/2, this->y() + (this->h() - HEIGHT)/2, WIDTH, HEIGHT, CORNER_RADIUS);
        this->addElement(this->bg);

        this->heading = new Aether::Text(this->bg->x() + X_PADDING, this->bg->y() + Y_PADDING, "|", HEADING_FONT_SIZE);
        this->addElement(this->heading);
        this->subheading = new Aether::Text(this->heading->x(), this->heading->y() + this->heading->h() + 10, "", SUBHEADING_FONT_SIZE);
        this->addElement(this->subheading);

        this->anim = new Aether::Animation(this->bg->x() + X_PADDING, this->bg->y() + this->bg->h() - Y_PADDING - 20, 40, 20);
        for (size_t i = 0; i < this->animFrames.size(); i++) {
            this->animFrames[i] = new Aether::Image(this->anim->x(), this->anim->y(), "romfs:/anim/infload/" + std::to_string(i+1) + ".png");
            this->animFrames[i]->setWH(40, 20);
            this->anim->addElement(this->animFrames[i]);
        }
        this->anim->setAnimateSpeed(50);
        this->addElement(this->anim);

        size_t barW = this->bg->w() - 2*(this->anim->w() + X_PADDING + 20);
        this->pbar = new Aether::RoundProgressBar(this->anim->x() + this->anim->w() + 20, this->anim->y() + this->anim->h()/2 - 6, barW);
        this->addElement(this->pbar);

        this->perc = new Aether::Text(this->pbar->x() + this->pbar->w() + 20, this->pbar->y() + this->pbar->h()/2, "|", SUBHEADING_FONT_SIZE);
        this->perc->setY(this->perc->y() - this->perc->h()/2);
        this->addElement(this->perc);

        this->setValue(0.0);
    }

    void ProgressBox::setHeadingText(const std::string & s) {
        this->heading->setString(s);
    }

    void ProgressBox::setSubheadingText(const std::string & s) {
        this->subheading->setString(s);
        if (this->subheading->texW() > this->bg->w() - 2*X_PADDING) {
            this->subheading->setW(this->bg->w() - 2*X_PADDING);
        }
    }

    void ProgressBox::setValue(float f) {
        this->pbar->setValue(f);
        this->perc->setString(Utils::truncateToDecimalPlace(std::to_string(Utils::roundToDecimalPlace(f, 0)), 0) + "%");
    }

    void ProgressBox::setBackgroundColour(const Aether::Colour & c) {
        this->bg->setColour(c);
    }

    void ProgressBox::setTextColour(const Aether::Colour & c) {
        this->heading->setColour(c);
        this->perc->setColour(c);
    }

    void ProgressBox::setMutedTextColour(const Aether::Colour & c) {
        this->subheading->setColour(c);
    }

    void ProgressBox::setBarBackgroundColour(const Aether::Colour & c) {
        this->pbar->setBackgroundColour(c);
    }

    void ProgressBox::setBarForegroundColour(const Aether::Colour & c) {
        this->pbar->setForegroundColour(c);
        for (size_t i = 0; i < this->animFrames.size(); i++) {
            this->animFrames[i]->setColour(c);
        }
    }
};