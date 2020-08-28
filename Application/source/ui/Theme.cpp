#include "ui/Theme.hpp"

std::string Theme::colourToString(const Colour c) {
    std::string str;
    switch (c) {
        case Theme::Colour::Red:
            str = "Red";
            break;

        case Theme::Colour::Orange:
            str = "Orange";
            break;

        case Theme::Colour::Yellow:
            str = "Yellow";
            break;

        case Theme::Colour::Green:
            str = "Green";
            break;

        case Theme::Colour::Blue:
            str = "Blue";
            break;

        case Theme::Colour::Purple:
            str = "Purple";
            break;

        case Theme::Colour::Pink:
            str = "Pink";
            break;
    }
    return str;
}

Theme::Theme() {
    this->bottomBG_ = Aether::Colour{25, 25, 25, 166};
    this->popupBG_ = Aether::Colour{30, 30, 30, 255};
    this->sideBG_ = Aether::Colour{17, 17, 17, 128};
    this->muted_ = Aether::Colour{120, 120, 120, 255};
    this->muted2_ = Aether::Colour{54, 54, 54, 255};
    this->FG_ = Aether::Colour{255, 255, 255, 255};
    this->setAccent(Colour::Blue);
    this->selected_ = Aether::Colour{100, 150, 255, 55};
}

void Theme::setAccent(Colour c) {
    switch (c) {
        case Colour::Red:
            this->accent_ = Aether::Colour{255, 25, 65, 255};
            break;

        case Colour::Orange:
            this->accent_ = Aether::Colour{255, 170, 30, 255};
            break;

        case Colour::Yellow:
            this->accent_ = Aether::Colour{255, 255, 50, 255};
            break;

        case Colour::Green:
            this->accent_ = Aether::Colour{50, 255, 100, 255};
            break;

        case Colour::Blue:
            this->accent_ = Aether::Colour{85, 255, 255, 255};
            break;

        case Colour::Purple:
            this->accent_ = Aether::Colour{150, 0, 225, 255};
            break;

        case Colour::Pink:
            this->accent_ = Aether::Colour{255, 125, 255, 255};
            break;
    }
}

Aether::Colour Theme::bottomBG() {
    return this->bottomBG_;
}

Aether::Colour Theme::popupBG() {
    return this->popupBG_;
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