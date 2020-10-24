#include <string>
#include <vector>

namespace NX {
    // Valid buttons which can form a key combination
    enum class Button {
        A = 0,
        B = 1,
        X = 2,
        Y = 3,
        LSTICK = 4,
        RSTICK = 5,
        L = 6,
        R = 7,
        ZL = 8,
        ZR = 9,
        PLUS = 10,
        MINUS = 11,
        DLEFT = 12,
        DUP = 13,
        DRIGHT = 14,
        DDOWN = 15
    };

    // Convert a single button to it's equivalent unicode character
    std::string buttonToCharacter(const Button);

    // Convert a key combination to a string
    std::string comboToString(const std::vector<Button> &);

    // Convert a key combination to a unicode string, using given character to split
    std::string comboToUnicodeString(const std::vector<Button> &, const std::string &);

    // Convert a string to a key combination (empty if any key is invalid)
    // Valid delimiters are ' ' and '+' and is not case sensitive
    std::vector<Button> stringToCombo(const std::string &);
};