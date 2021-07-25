#include "Application.hpp"
#include "lang/Lang.hpp"
#include "ui/frame/settings/Frame.hpp"
#include "utils/NX.hpp"

// Padding either side of list
#define LIST_PADDING 10

namespace Frame::Settings {
    Frame::Frame(Main::Application * a) : Aether::Container(350, 0, 930, 720) {
        this->app = a;

        // Create and position empty list
        this->list = new Aether::List(this->x() + LIST_PADDING, this->y(), this->w() - 2 * LIST_PADDING, this->h());
        this->list->setScrollBarColour(this->app->theme()->muted2());
        this->addElement(this->list);
        this->setFocused(this->list);
        this->list->addElement(new Aether::ListSeparator());
    }

    void Frame::addButton(const std::string & str, std::function<void()> f) {
        Aether::ListButton * btn = new Aether::ListButton(str, f);
        btn->setColours(this->app->theme()->muted2(), this->app->theme()->FG());
        this->list->addElement(btn);
    }

    void Frame::addComment(const std::string & str) {
        Aether::ListComment * cmt = new Aether::ListComment(str);
        cmt->setTextColour(this->app->theme()->muted());
        this->list->addElement(cmt);
    }

    void Frame::addToggle(const std::string & str, std::function<bool()> get, std::function<void(bool)> set) {
        // Get current value
        bool b = get();

        // Create element and set appropriate text/colour based on above value
        Aether::ListOption * opt = new Aether::ListOption(str, (b ? "Common.Yes"_lang : "Common.No"_lang), nullptr);
        opt->onPress([this, opt, get, set]() {
            // Set opposite value
            bool b = get();
            set(!b);

            // Update element to match new state
            b = get();
            opt->setValue(b ? "Common.Yes"_lang : "Common.No"_lang);
            opt->setValueColour(b ? this->app->theme()->accent() : this->app->theme()->muted());
        });
        opt->setHintColour(this->app->theme()->FG());
        opt->setLineColour(this->app->theme()->muted2());
        opt->setValueColour(b ? this->app->theme()->accent() : this->app->theme()->muted());

        this->list->addElement(opt);
    }

    bool Frame::getNumberInput(int & val, const std::string & heading = "", const std::string & sub = "", const bool neg = false) {
        // Configure and open numpad
        Utils::NX::Numpad numpad;
        numpad.value = val;
        numpad.maxDigits = 5;
        numpad.allowNegative = neg;
        numpad.allowDecimal = false;
        numpad.heading = heading;
        numpad.subHeading = sub;
        if (Utils::NX::getUserInput(numpad)) {
            val = numpad.value;
            return true;
        }
        return false;
    }
};
