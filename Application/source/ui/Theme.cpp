#include "Theme.hpp"

namespace Main {
    Theme::Theme() {
        this->bottomBG_ = Aether::Colour{25, 25, 25, 166};
        this->sideBG_ = Aether::Colour{17, 17, 17, 128};
        this->muted_ = Aether::Colour{120, 120, 120, 255};
        this->muted2_ = Aether::Colour{54, 54, 54, 255};
        this->FG_ = Aether::Colour{255, 255, 255, 255};


        this->accent_ = Aether::Colour{85, 255, 255, 255};
        this->selected_ = Aether::Colour{100, 150, 255, 55};
    }

    Aether::Colour Theme::bottomBG() {
        return this->bottomBG_;
    }

    Aether::Colour Theme::sideBG() {
        return this->sideBG_;
    }

    Aether::Colour Theme::muted() {
        return this->muted_;
    }

    Aether::Colour Theme::muted2() {
        return this->muted2_;
    }

    Aether::Colour Theme::FG() {
        return this->FG_;
    }

    Aether::Colour Theme::accent() {
        return this->accent_;
    }

    Aether::Colour Theme::selected() {
        return this->selected_;
    }
};