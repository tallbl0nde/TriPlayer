#include "Theme.hpp"

namespace Main {
    Theme::Theme() {
        this->BG_ = Aether::Colour{20, 20, 35, 255};
        this->sideBG_ = Aether::Colour{30, 30, 45, 255};
        this->bottomBG_ = Aether::Colour{40, 40, 70, 255};
        this->text_ = Aether::Colour{255, 255, 255, 255};
        this->heading_ = Aether::Colour{225, 225, 255, 255};
        this->mutedText_ = Aether::Colour{140, 140, 170, 255};
        this->mutedLine_ = Aether::Colour{60, 60, 100, 255};
        this->accent_ = Aether::Colour{85, 255, 255, 255};
        this->selected_ = Aether::Colour{100, 150, 255, 55};
    }

    Aether::Colour Theme::BG() {
        return this->BG_;
    }

    Aether::Colour Theme::sideBG() {
        return this->sideBG_;
    }

    Aether::Colour Theme::bottomBG() {
        return this->bottomBG_;
    }

    Aether::Colour Theme::text() {
        return this->text_;
    }

    Aether::Colour Theme::heading() {
        return this->heading_;
    }

    Aether::Colour Theme::mutedText() {
        return this->mutedText_;
    }

    Aether::Colour Theme::mutedLine() {
        return this->mutedLine_;
    }

    Aether::Colour Theme::accent() {
        return this->accent_;
    }

    Aether::Colour Theme::selected() {
        return this->selected_;
    }
};