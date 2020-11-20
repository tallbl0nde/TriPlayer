#include "Application.hpp"
#include "lang/Lang.hpp"
#include "Paths.hpp"
#include "ui/frame/settings/About.hpp"
#include "ui/frame/settings/AppAdvanced.hpp"
#include "ui/frame/settings/AppAppearance.hpp"
#include "ui/frame/settings/AppGeneral.hpp"
#include "ui/frame/settings/AppMetadata.hpp"
#include "ui/frame/settings/AppSearch.hpp"
#include "ui/frame/settings/SysGeneral.hpp"
#include "ui/frame/settings/SysMP3.hpp"
#include "ui/screen/Settings.hpp"

// Pixels between each entry in the sidebar
#define SIDE_PADDING 20
#define SIDEBAR_SEP 5

namespace Screen {
    Settings::Settings(Main::Application * a) : Screen(a) {
        // Call backCallback() when B is pressed
        this->onButtonPress(Aether::Button::B, [this]() {
            this->backCallback();
        });
    }

    void Settings::backCallback() {
        this->app->popScreen();
    }

    void Settings::setupNew() {
        this->buttonBack->setActivated(false);
        this->buttonAppGeneral->setActivated(false);
        this->buttonAppAppearance->setActivated(false);
        this->buttonAppMetadata->setActivated(false);
        this->buttonAppSearch->setActivated(false);
        this->buttonAppAdvanced->setActivated(false);
        this->buttonSysGeneral->setActivated(false);
        this->buttonSysMP3->setActivated(false);
        this->buttonAbout->setActivated(false);
        this->removeElement(this->frame);
    }

    void Settings::setupAppGeneral() {
        this->buttonAppGeneral->setActivated(true);
        this->frame = new Frame::Settings::AppGeneral(this->app);
        this->addElement(this->frame);
    }

    void Settings::setupAppAppearance() {
        this->buttonAppAppearance->setActivated(true);
        this->frame = new Frame::Settings::AppAppearance(this->app);
        this->addElement(this->frame);
    }

    void Settings::setupAppMetadata() {
        this->buttonAppMetadata->setActivated(true);
        this->frame = new Frame::Settings::AppMetadata(this->app);
        this->addElement(this->frame);
    }

    void Settings::setupAppSearch() {
        this->buttonAppSearch->setActivated(true);
        this->frame = new Frame::Settings::AppSearch(this->app);
        this->addElement(this->frame);
    }

    void Settings::setupAppAdvanced() {
        this->buttonAppAdvanced->setActivated(true);
        this->frame = new Frame::Settings::AppAdvanced(this->app);
        this->addElement(this->frame);
    }

    void Settings::setupSysGeneral() {
        this->buttonSysGeneral->setActivated(true);
        this->frame = new Frame::Settings::SysGeneral(this->app);
        this->addElement(this->frame);
    }

    void Settings::setupSysMP3() {
        this->buttonSysMP3->setActivated(true);
        this->frame = new Frame::Settings::SysMP3(this->app);
        this->addElement(this->frame);
    }

    void Settings::setupAbout() {
        this->buttonAbout->setActivated(true);
        this->frame = new Frame::Settings::About(this->app);
        this->addElement(this->frame);
    }

    void Settings::updateColours() {
        if (this->isLoaded) {
            this->buttonBack->setActiveColour(this->app->theme()->accent());
            this->buttonAppGeneral->setActiveColour(this->app->theme()->accent());
            this->buttonAppAppearance->setActiveColour(this->app->theme()->accent());
            this->buttonAppMetadata->setActiveColour(this->app->theme()->accent());
            this->buttonAppSearch->setActiveColour(this->app->theme()->accent());
            this->buttonAppAdvanced->setActiveColour(this->app->theme()->accent());
            this->buttonSysGeneral->setActiveColour(this->app->theme()->accent());
            this->buttonSysMP3->setActiveColour(this->app->theme()->accent());
            this->buttonAbout->setActiveColour(this->app->theme()->accent());
            this->updateDot->setColour(this->app->theme()->accent());
        }
    }

    void Settings::update(uint32_t dt) {
        Screen::update(dt);
        this->updateDot->setHidden(!this->app->hasUpdate());
    }

    void Settings::addHeading(const std::string & str) {
        // Container
        Aether::Element * container = new Aether::Element();
        this->sidebarList->addElement(container);

        // Text
        Aether::Text * text = new Aether::Text(container->x() + container->w()*0.05, container->y() + 20, str, 20);
        text->setColour(this->app->theme()->muted());
        container->addElement(text);

        // Line
        Aether::Rectangle * sep = new Aether::Rectangle(container->x() + container->w()*0.05, text->y() + text->h() + 10, container->w()*0.9, 1);
        sep->setColour(this->app->theme()->muted2());
        container->addElement(sep);

        container->setH(sep->y() - container->y() + 10);
    }

    void Settings::onLoad() {
        Screen::onLoad();

        // Create background textures
        this->bg = new Aether::Image(0, 0, "romfs:/bg/main.png");
        this->addElement(this->bg);
        this->sidebarGradient = new Aether::Image(350, 15, "romfs:/misc/sidegradient.png");
        this->sidebarGradient->setColour(Aether::Colour{255, 255, 255, 200});
        this->addElement(this->sidebarGradient);
        this->sidebarBg = new Aether::Rectangle(0, 0, 350, 720);
        this->sidebarBg->setColour(this->app->theme()->sideBG());
        this->addElement(this->sidebarBg);

        // Sidebar contains a list of SideButtons
        this->sidebarList = new Aether::List(this->sidebarBg->x() + SIDE_PADDING, this->sidebarBg->y(), this->sidebarBg->w() - 2*SIDE_PADDING, this->sidebarBg->h(), Aether::Padding::FitScrollbar);
        this->sidebarList->setShowScrollBar(false);

        // Back
        this->buttonBack = new CustomElm::SideButton(0, 0, 100);
        this->buttonBack->setText("Settings.BackToLibrary"_lang);
        this->buttonBack->setCallback([this]() {
            this->backCallback();
        });
        this->buttonBack->setActiveColour(this->app->theme()->accent());
        this->buttonBack->setInactiveColour(this->app->theme()->FG());
        this->sidebarList->addElement(this->buttonBack);
        this->sidebarList->addElement(new Aether::ListSeparator(SIDEBAR_SEP));

        // Application (general)
        this->addHeading("Settings.Application"_lang);
        this->buttonAppGeneral = new CustomElm::SideButton(0, 0, 100);
        this->buttonAppGeneral->setText("Settings.General"_lang);
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
        this->buttonAppAppearance->setText("Settings.Appearance"_lang);
        this->buttonAppAppearance->setCallback([this]() {
            this->setupNew();
            this->setupAppAppearance();
        });
        this->buttonAppAppearance->setActiveColour(this->app->theme()->accent());
        this->buttonAppAppearance->setInactiveColour(this->app->theme()->FG());
        this->sidebarList->addElement(this->buttonAppAppearance);
        this->sidebarList->addElement(new Aether::ListSeparator(SIDEBAR_SEP));

        // Application (appearance)
        this->buttonAppMetadata = new CustomElm::SideButton(0, 0, 100);
        this->buttonAppMetadata->setText("Settings.Metadata"_lang);
        this->buttonAppMetadata->setCallback([this]() {
            this->setupNew();
            this->setupAppMetadata();
        });
        this->buttonAppMetadata->setActiveColour(this->app->theme()->accent());
        this->buttonAppMetadata->setInactiveColour(this->app->theme()->FG());
        this->sidebarList->addElement(this->buttonAppMetadata);
        this->sidebarList->addElement(new Aether::ListSeparator(SIDEBAR_SEP));

        // Application (search)
        this->buttonAppSearch = new CustomElm::SideButton(0, 0, 100);
        this->buttonAppSearch->setText("Settings.Search"_lang);
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
        this->buttonAppAdvanced->setText("Settings.Advanced"_lang);
        this->buttonAppAdvanced->setCallback([this]() {
            this->setupNew();
            this->setupAppAdvanced();
        });
        this->buttonAppAdvanced->setActiveColour(this->app->theme()->accent());
        this->buttonAppAdvanced->setInactiveColour(this->app->theme()->FG());
        this->sidebarList->addElement(this->buttonAppAdvanced);
        this->sidebarList->addElement(new Aether::ListSeparator(SIDEBAR_SEP));

        // Sysmodule (general)
        this->addHeading("Settings.Sysmodule"_lang);
        this->buttonSysGeneral = new CustomElm::SideButton(0, 0, 100);
        this->buttonSysGeneral->setText("Settings.General"_lang);
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
        this->buttonSysMP3->setText("Settings.MP3"_lang);
        this->buttonSysMP3->setCallback([this]() {
            this->setupNew();
            this->setupSysMP3();
        });
        this->buttonSysMP3->setActiveColour(this->app->theme()->accent());
        this->buttonSysMP3->setInactiveColour(this->app->theme()->FG());
        this->sidebarList->addElement(this->buttonSysMP3);
        this->sidebarList->addElement(new Aether::ListSeparator(SIDEBAR_SEP));

        // About
        this->addHeading("Settings.Miscellaneous"_lang);
        this->buttonAbout = new CustomElm::SideButton(0, 0, 100);
        this->buttonAbout->setText("Settings.About"_lang);
        this->buttonAbout->setCallback([this]() {
            this->setupNew();
            this->setupAbout();
        });
        this->buttonAbout->setActiveColour(this->app->theme()->accent());
        this->buttonAbout->setInactiveColour(this->app->theme()->FG());
        this->sidebarList->addElement(this->buttonAbout);
        this->updateDot = new Aether::Ellipse(this->buttonAbout->x() + this->buttonAbout->w() - this->buttonAbout->h()/2 - 8, this->buttonAbout->y() + this->buttonAbout->h()/2 - 8, 16);
        this->updateDot->setColour(this->app->theme()->accent());
        this->buttonAbout->addElement(this->updateDot);

        this->addElement(this->sidebarList);

        // Ensure we have loaded the sysmodule config
        this->app->config()->prepareSys(Path::Sys::ConfigFile);

        // Start on first tab!
        this->frame = nullptr;
        this->setupNew();
        this->setupAppGeneral();
    }

    void Settings::onUnload() {
        Screen::onUnload();
        this->removeElement(this->frame);
        this->removeElement(this->bg);
        this->removeElement(this->sidebarGradient);
        this->removeElement(this->sidebarBg);
        this->removeElement(this->sidebarList);
    }
};