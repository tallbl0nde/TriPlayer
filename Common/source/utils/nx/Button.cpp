#include <cstring>
#include "utils/nx/Button.hpp"

namespace NX {
    std::string buttonToCharacter(const Button b) {
        switch (b) {
            case Button::A:
                return "\uE0E0";
                break;

            case Button::B:
                return "\uE0E1";
                break;

            case Button::X:
                return "\uE0E2";
                break;

            case Button::Y:
                return "\uE0E3";
                break;

            case Button::LSTICK:
                return "\uE104";
                break;

            case Button::RSTICK:
                return "\uE105";
                break;

            case Button::L:
                return "\uE0E4";
                break;

            case Button::R:
                return "\uE0E5";
                break;

            case Button::ZL:
                return "\uE0E6";
                break;

            case Button::ZR:
                return "\uE0E7";
                break;

            case Button::PLUS:
                return "\uE0F1";
                break;

            case Button::MINUS:
                return "\uE0F2";
                break;

            case Button::DLEFT:
                return "\uE0ED";
                break;

            case Button::DUP:
                return "\uE0EB";
                break;

            case Button::DRIGHT:
                return "\uE0EE";
                break;

            case Button::DDOWN:
                return "\uE0EC";
                break;
        }
        return "?";
    }

    std::string comboToString(const std::vector<Button> & combo) {
        std::string str = "";

        // Iterate over each button forming combination
        for (size_t i = 0; i < combo.size(); i++) {
            // Convert to string and append
            switch (combo[i]) {
                case Button::A:
                    str += "A";
                    break;

                case Button::B:
                    str += "B";
                    break;

                case Button::X:
                    str += "X";
                    break;

                case Button::Y:
                    str += "Y";
                    break;

                case Button::LSTICK:
                    str += "LSTICK";
                    break;

                case Button::RSTICK:
                    str += "RSTICK";
                    break;

                case Button::L:
                    str += "L";
                    break;

                case Button::R:
                    str += "R";
                    break;

                case Button::ZL:
                    str += "ZL";
                    break;

                case Button::ZR:
                    str += "ZR";
                    break;

                case Button::PLUS:
                    str += "PLUS";
                    break;

                case Button::MINUS:
                    str += "MINUS";
                    break;

                case Button::DLEFT:
                    str += "DLEFT";
                    break;

                case Button::DUP:
                    str += "DUP";
                    break;

                case Button::DRIGHT:
                    str += "DRIGHT";
                    break;

                case Button::DDOWN:
                    str += "DDOWN";
                    break;
            }

            // If not the last one append a plus
            if (i < combo.size() - 1) {
                str += "+";
            }
        }

        return str;
    }

    std::string comboToUnicodeString(const std::vector<Button> & combo, const std::string & delim) {
        std::string str = "";
        for (size_t i = 0; i < combo.size(); i++) {
            str += buttonToCharacter(combo[i]);
            if (i < combo.size() - 1) {
                str += delim;
            }
        }
        return str;
    }

    constexpr char delims[] = " +";     // Supported delimiters
    std::vector<Button> stringToCombo(const std::string & str) {
        // Vector to return
        std::vector<Button> combo;

        // Ensure we have a string
        if (str.length() > 0) {
            // Split on delimiters
            std::vector<std::string> tokens;
            char * copy = strdup(str.c_str());
            char * tok = std::strtok(copy, delims);
            while (tok != nullptr) {
                tokens.push_back(std::string(tok));
                tok = std::strtok(nullptr, delims);
            }
            free(copy);

            // Convert each word to upper case (only handles ASCII but that's all we support)
            for (std::string & token : tokens) {
                std::transform(token.begin(), token.end(), token.begin(), [](unsigned char c) {
                    return std::toupper(c);
                });
            }

            // Finally compare each token to see if it matches a button
            // Note that we return an empty vector even if only one button is wrong
            for (std::string & token : tokens) {
                if (token == "A") {
                    combo.push_back(Button::A);

                } else if (token == "B") {
                    combo.push_back(Button::B);

                } else if (token == "X") {
                    combo.push_back(Button::X);

                } else if (token == "Y") {
                    combo.push_back(Button::Y);

                } else if (token == "LSTICK") {
                    combo.push_back(Button::LSTICK);

                } else if (token == "RSTICK") {
                    combo.push_back(Button::RSTICK);

                } else if (token == "L") {
                    combo.push_back(Button::L);

                } else if (token == "R") {
                    combo.push_back(Button::R);

                } else if (token == "ZL") {
                    combo.push_back(Button::ZL);

                } else if (token == "ZR") {
                    combo.push_back(Button::ZR);

                } else if (token == "PLUS") {
                    combo.push_back(Button::PLUS);

                } else if (token == "MINUS") {
                    combo.push_back(Button::MINUS);

                } else if (token == "DLEFT") {
                    combo.push_back(Button::DLEFT);

                } else if (token == "DUP") {
                    combo.push_back(Button::DUP);

                } else if (token == "DRIGHT") {
                    combo.push_back(Button::DRIGHT);

                } else if (token == "DDOWN") {
                    combo.push_back(Button::DDOWN);

                } else {
                    combo.clear();
                    break;
                }
            }
        }

        return combo;
    }
};