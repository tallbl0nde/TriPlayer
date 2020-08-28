#include "Application.hpp"
#include "ui/frame/settings/Frame.hpp"

// Padding either side of list
#define LIST_PADDING 50

namespace Frame::Settings {
    Frame::Frame(Main::Application * a) : Aether::Container(310, 0, 970, 720) {
        this->app = a;

        // Create and position empty list
        this->list = new Aether::List(this->x() + LIST_PADDING, this->y(), this->w() - 2 * LIST_PADDING, this->h());
        this->addElement(this->list);
        this->setFocused(this->list);
        this->list->addElement(new Aether::ListSeparator());
    }

    void Frame::addToggle(const std::string & str, std::function<bool()> get, std::function<void(bool)> set) {
        // Get current value
        bool b = get();

        // Create element and set appropriate text/colour based on above value
        Aether::ListOption * opt = new Aether::ListOption(str, (b ? "Yes" : "No"), nullptr);
        opt->setCallback([this, opt, get, set]() {
            // Set opposite value
            bool b = get();
            set(!b);

            // Update element to match new state
            b = get();
            opt->setValue(b ? "Yes" : "No");
            opt->setValueColour(b ? this->app->theme()->accent() : this->app->theme()->muted());
        });
        opt->setHintColour(this->app->theme()->FG());
        opt->setLineColour(this->app->theme()->muted2());
        opt->setValueColour(b ? this->app->theme()->accent() : this->app->theme()->muted());

        this->list->addElement(opt);
    }

    void Frame::addComment(const std::string & str) {
        Aether::ListComment * cmt = new Aether::ListComment(str);
        cmt->setTextColour(this->app->theme()->muted());
        this->list->addElement(cmt);
    }
};