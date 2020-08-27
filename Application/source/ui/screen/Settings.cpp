#include "Application.hpp"
#include "ui/screen/Settings.hpp"

// Padding either side of list
#define LIST_PADDING 50
// Pixels between each entry in the sidebar
#define SIDEBAR_SEP 5

namespace Screen {
    Settings::Settings(Main::Application * a) : Aether::Screen() {
        this->app = a;

        // Call backCallback() when B is pressed
        this->onButtonPress(Aether::Button::B, [this]() {
            this->backCallback();
        });
    }

    void Settings::backCallback() {
        this->app->popScreen();
    }

    void Settings::setupNew() {
        this->buttonAppGeneral->setActivated(false);
        this->buttonAppAppearance->setActivated(false);
        this->buttonAppSearch->setActivated(false);
        this->buttonAppAdvanced->setActivated(false);
        this->buttonSysGeneral->setActivated(false);
        this->buttonSysMP3->setActivated(false);
        this->mainList->removeAllElements();
    }

    void Settings::setupAppGeneral() {
        this->buttonAppGeneral->setActivated(true);

        // Temporary pointers to elements
        Aether::ListOption * opt;
        Aether::ListComment * cmt;
        this->mainList->addElement(new Aether::ListSeparator());

        // General::confirm_clear_queue
        bool b = this->app->config()->confirmClearQueue();
        opt = new Aether::ListOption("Confirm Clearing Queue", (b ? "Yes": "No"), nullptr);
        opt->setCallback([this, opt]() {
            bool b = this->app->config()->confirmClearQueue();
            this->app->config()->setConfirmClearQueue(!b);
            opt->setValue(!b ? "Yes" : "No");
            opt->setValueColour((!b ? this->app->theme()->accent() : this->app->theme()->muted()));
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), (b ? this->app->theme()->accent() : this->app->theme()->muted()));
        this->mainList->addElement(opt);

        cmt = new Aether::ListComment("Show a popup confirming that you want to clear the queue when playing a new song.");
        cmt->setTextColour(this->app->theme()->muted());
        this->mainList->addElement(cmt);

        // General::confirm_exit
        b = this->app->config()->confirmExit();
        opt = new Aether::ListOption("Confirm Exit", (b ? "Yes": "No"), nullptr);
        opt->setCallback([this, opt]() {
            bool b = this->app->config()->confirmExit();
            this->app->config()->setConfirmExit(!b);
            opt->setValue(!b ? "Yes" : "No");
            opt->setValueColour((!b ? this->app->theme()->accent() : this->app->theme()->muted()));
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), (b ? this->app->theme()->accent() : this->app->theme()->muted()));
        this->mainList->addElement(opt);

        cmt = new Aether::ListComment("Show a confirmation dialog when going to exit the app.");
        cmt->setTextColour(this->app->theme()->muted());
        this->mainList->addElement(cmt);

        // General::initial_frame
    }

    void Settings::setupAppAppearance() {
        this->buttonAppAppearance->setActivated(true);
    }

    void Settings::setupAppSearch() {
        this->buttonAppSearch->setActivated(true);
    }

    void Settings::setupAppAdvanced() {
        this->buttonAppAdvanced->setActivated(true);
    }

    void Settings::setupSysGeneral() {
        this->buttonSysGeneral->setActivated(true);
    }

    void Settings::setupSysMP3() {
        this->buttonSysMP3->setActivated(true);
    }

    void Settings::onLoad() {
        // Create background textures
        this->bg = new Aether::Image(0, 0, "romfs:/bg/main.png");
        this->addElement(this->bg);
        this->sidebarGradient = new Aether::Image(310, 15, "romfs:/misc/sidegradient.png");
        this->sidebarGradient->setColour(Aether::Colour{255, 255, 255, 200});
        this->addElement(this->sidebarGradient);
        this->sidebarBg = new Aether::Rectangle(0, 0, 310, 720);
        this->sidebarBg->setColour(this->app->theme()->sideBG());
        this->addElement(this->sidebarBg);

        // Sidebar contains a list of SideButtons
        this->sidebarList = new Aether::List(this->sidebarBg->x(), this->sidebarBg->y(), this->sidebarBg->w(), this->sidebarBg->h(), Aether::Padding::FitScrollbar);

        // Application (general)
        this->buttonAppGeneral = new CustomElm::SideButton(0, 0, 100);
        this->buttonAppGeneral->setText("General");
        this->buttonAppGeneral->setCallback([this]() {
            this->setupNew();
            this->setupAppGeneral();
        });
        this->buttonAppGeneral->setActiveColour(this->app->theme()->accent());
        this->buttonAppGeneral->setInactiveColour(this->app->theme()->FG());
        this->sidebarList->addElement(this->buttonAppGeneral);
        this->sidebarList->addElement(new Aether::ListSeparator(SIDEBAR_SEP));

        // Application (appearance)
        this->buttonAppAppearance = new CustomElm::SideButton(0, 0, 100);
        this->buttonAppAppearance->setText("Appearance");
        this->buttonAppAppearance->setCallback([this]() {
            this->setupNew();
            this->setupAppAppearance();
        });
        this->buttonAppAppearance->setActiveColour(this->app->theme()->accent());
        this->buttonAppAppearance->setInactiveColour(this->app->theme()->FG());
        this->sidebarList->addElement(this->buttonAppAppearance);
        this->sidebarList->addElement(new Aether::ListSeparator(SIDEBAR_SEP));

        // Application (search)
        this->buttonAppSearch = new CustomElm::SideButton(0, 0, 100);
        this->buttonAppSearch->setText("Search");
        this->buttonAppSearch->setCallback([this]() {
            this->setupNew();
            this->setupAppSearch();
        });
        this->buttonAppSearch->setActiveColour(this->app->theme()->accent());
        this->buttonAppSearch->setInactiveColour(this->app->theme()->FG());
        this->sidebarList->addElement(this->buttonAppSearch);
        this->sidebarList->addElement(new Aether::ListSeparator(SIDEBAR_SEP));

        // Application (advanced)
        this->buttonAppAdvanced = new CustomElm::SideButton(0, 0, 100);
        this->buttonAppAdvanced->setText("Advanced");
        this->buttonAppAdvanced->setCallback([this]() {
            this->setupNew();
            this->setupAppAdvanced();
        });
        this->buttonAppAdvanced->setActiveColour(this->app->theme()->accent());
        this->buttonAppAdvanced->setInactiveColour(this->app->theme()->FG());
        this->sidebarList->addElement(this->buttonAppAdvanced);
        this->sidebarList->addElement(new Aether::ListSeparator(SIDEBAR_SEP));

        // Sysmodule (general)
        this->buttonSysGeneral = new CustomElm::SideButton(0, 0, 100);
        this->buttonSysGeneral->setText("General");
        this->buttonSysGeneral->setCallback([this]() {
            this->setupNew();
            this->setupSysGeneral();
        });
        this->buttonSysGeneral->setActiveColour(this->app->theme()->accent());
        this->buttonSysGeneral->setInactiveColour(this->app->theme()->FG());
        this->sidebarList->addElement(this->buttonSysGeneral);
        this->sidebarList->addElement(new Aether::ListSeparator(SIDEBAR_SEP));

        // Sysmodule (MP3)
        this->buttonSysMP3 = new CustomElm::SideButton(0, 0, 100);
        this->buttonSysMP3->setText("MP3");
        this->buttonSysMP3->setCallback([this]() {
            this->setupNew();
            this->setupSysMP3();
        });
        this->buttonSysMP3->setActiveColour(this->app->theme()->accent());
        this->buttonSysMP3->setInactiveColour(this->app->theme()->FG());
        this->sidebarList->addElement(this->buttonSysMP3);

        this->addElement(this->sidebarList);

        // Create the list for the main section of the screen
        this->mainList = new Aether::List(this->sidebarBg->x() + this->sidebarBg->w() + LIST_PADDING, this->y(), this->w() - this->sidebarBg->w() - 2*LIST_PADDING, this->h());
        this->addElement(this->mainList);

        // Start on first tab!
        this->setupNew();
        this->setupAppGeneral();
    }

    void Settings::onUnload() {
        this->removeElement(this->bg);
        this->removeElement(this->sidebarGradient);
        this->removeElement(this->sidebarBg);
        this->removeElement(this->sidebarList);
        this->removeElement(this->mainList);
    }
};