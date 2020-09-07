#include <numbers>
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
    this->accentColour = Colour::Blue;
    this->bottomBG_ = Aether::Colour{25, 25, 25, 166};
    this->popupBG_ = Aether::Colour{30, 30, 30, 255};
    this->sideBG_ = Aether::Colour{17, 17, 17, 128};
    this->muted_ = Aether::Colour{120, 120, 120, 255};
    this->muted2_ = Aether::Colour{54, 54, 54, 255};
    this->FG_ = Aether::Colour{255, 255, 255, 255};
    this->selected_ = Aether::Colour{100, 150, 255, 55};
}

void Theme::setAccent(Colour c) {
    this->accentColour = c;
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
    switch (this->accentColour) {
        case Colour::Red:
            return Aether::Colour{255, 25, 65, 255};
            break;

        case Colour::Orange:
            return Aether::Colour{255, 170, 30, 255};
            break;

        case Colour::Yellow:
            return Aether::Colour{255, 255, 50, 255};
            break;

        case Colour::Green:
            return Aether::Colour{50, 255, 100, 255};
            break;

        case Colour::Blue:
            return Aether::Colour{85, 255, 255, 255};
            break;

        case Colour::Purple:
            return Aether::Colour{150, 0, 225, 255};
            break;

        case Colour::Pink:
            return Aether::Colour{255, 125, 255, 255};
            break;
    }
    return Aether::Colour{255, 255, 255, 255};
}

Aether::Colour Theme::selected() {
    return this->selected_;
}

std::function<Aether::Colour(uint32_t)> Theme::highlightFunc() {
    // Copy colours in case object is destroyed
    Aether::Colour h1;
    Aether::Colour h2;
    switch (this->accentColour) {
        case Colour::Red:
            h1 = Aether::Colour{150, 10, 10, 255};
            h2 = Aether::Colour{255, 55, 65, 255};
            break;

        case Colour::Orange:

            break;

        case Colour::Yellow:

            break;

        case Colour::Green:

            break;

        case Colour::Blue:
            h1 = Aether::Colour{0, 150, 190, 255};
            h2 = Aether::Colour{0, 250, 250, 255};
            break;

        case Colour::Purple:

            break;

        case Colour::Pink:

            break;
    }

    return [h1, h2](uint32_t t) {
        Aether::Colour col = {0, 0, 0, 0};
        col.r = h1.r + ((h2.r - h1.r) * (0.5 * sin(1.8 * std::numbers::pi * (t/1000.0)) + 0.5));
        col.g = h1.g + ((h2.g - h1.g) * (0.5 * sin(1.8 * std::numbers::pi * (t/1000.0)) + 0.5));
        col.b = h1.b + ((h2.b - h1.b) * (0.5 * sin(1.8 * std::numbers::pi * (t/1000.0)) + 0.5));
        col.a = h1.a + ((h2.a - h1.a) * (0.5 * sin(1.8 * std::numbers::pi * (t/1000.0)) + 0.5));
        return col;
    };
}