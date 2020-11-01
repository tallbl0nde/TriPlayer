#include "Application.hpp"
#include "ui/frame/settings/About.hpp"

namespace Frame::Settings {
    About::About(Main::Application * a) : Frame(a) {
        // Banner
        Aether::Image * image = new Aether::Image(0, 0, "romfs:/misc/banner.png");
        Aether::Element * container = new Aether::Element(0, 0, 100, image->h() + 30);
        this->list->addElement(container);
        image->setXY(container->x() + (container->w() - image->w())/2, container->y());
        container->addElement(image);
        this->list->addElement(new Aether::ListSeparator(50));

        // Version number
        Aether::Text * text = new Aether::Text(0, 0, "Version " + std::string(VER_STRING), 24);
        text->setColour(this->app->theme()->FG());
        text->setXY(image->x() + 172, image->y() + 92);
        container->addElement(text);

        // Copyright
        text = new Aether::Text(0, 0, "Copyright © 2020 tallbl0nde", 20);
        text->setXY(container->x() + (container->w() - text->w())/2, container->y() + container->h() - text->h());
        text->setColour(this->app->theme()->FG());
        container->addElement(text);

        // Check for Updates button
        this->addButton("Check for Updates", [this]() {
            this->app->pushScreen();
            this->app->setScreen(Main::ScreenID::Update);
        });
        if (this->app->hasUpdate()) {
            this->addComment("An update is available!");
        }
        this->list->addElement(new Aether::ListSeparator());

        // Support heading
        container = new Aether::Element(0, 0, 100, 30);
        this->list->addElement(container);
        text = new Aether::Text(container->x() + 10, container->y(), "Support", 24);
        text->setColour(this->app->theme()->FG());
        container->addElement(text);
        this->addComment("If TriPlayer has helped you enjoy your music on your Switch, I would appreciate your support through Ko-fi: https://ko-fi.com/tallbl0nde");
        this->list->addElement(new Aether::ListSeparator());

        // License Heading
        container = new Aether::Element(0, 0, 100, 30);
        this->list->addElement(container);
        text = new Aether::Text(container->x() + 10, container->y(), "Software Libraries", 24);
        text->setColour(this->app->theme()->FG());
        container->addElement(text);
        this->addComment("I would like to thank the contributors of the following open source libraries which are used within TriPlayer:");

        // Licenses
        this->addComment("Aether\nCopyright © 2020 tallbl0nde\nMIT License\nhttps://github.com/tallbl0nde/Aether");
        this->addComment("AVIR Image Resizing Algorithm\nCopyright © 2015-2020 Aleksey Vaneev\nMIT License\nhttps://github.com/avaneev/avir");
        this->addComment("Diff Template Library\nCopyright © 2015 cubicdaiya\nBSD License\nhttps://github.com/cubicdaiya/dtl");
        this->addComment("JSON for Modern C++\nCopyright © 2013-2020 Niels Lohmann\nMIT License\nhttps://github.com/nlohmann/json");
        this->addComment("libcURL\nCopyright © 1996-2020 Daniel Stenberg\nMIT/X Derivate License\nhttps://curl.haxx.se");
        this->addComment("libmpg123\nLGPL 2.1 License\nhttps://www.mpg123.de");
        this->addComment("libnx\nCopyright © 2017-2020 libnx Authors\nISC License\nhttps://github.com/switchbrew/libnx");
        this->addComment("minIni\nCopyright © compuphase\nApache 2 License\nhttps://github.com/compuphase/minIni");
        this->addComment("SDL2_gfx Extensions\nCopyright © 2018 Richard T. Russell\nzlib License\nhttps://github.com/rtrussell/BBCSDL");
        this->addComment("Splash\nCopyright © 2020 Google\nApache 2 License\nhttps://github.com/tallbl0nde/Splash");
        this->addComment("SQLite\nPublic Domain\nhttps://www.sqlite.org");
        this->addComment("SQLite-okapi-bm25\nCopyright © 2014 Radford Smith\nMIT License\nhttps://github.com/rads/sqlite-okapi-bm25");
        this->addComment("zziplib\nCopyright © 2000-2020 Guido Draheim\nGPL 2 License\nhttps://github.com/gdraheim/zziplib");
    }
}