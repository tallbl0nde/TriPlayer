#include "Application.hpp"
#include "lang/Lang.hpp"
#include "Paths.hpp"
#include "ui/screen/Update.hpp"
#include "Updater.hpp"
#include "utils/FS.hpp"
#include "utils/Utils.hpp"
#include "utils/Zip.hpp"

// Font sizes
constexpr unsigned int headingFontSize = 24;
constexpr unsigned int bodyFontSize = 20;

namespace Screen {
    Update::Update(Main::Application * a) : Screen(a) {
        // Exit only once thread is done
        this->onButtonPress(Aether::Button::B, [this](){
            if (this->threadDone) {
                this->app->popScreen();
            }
        });
    }

    void Update::createNewMsgbox() {
        delete this->msgbox;
        this->msgbox = new Aether::MessageBox();
        this->msgbox->onButtonPress(Aether::Button::B, nullptr);
        this->msgbox->setRectangleColour(this->app->theme()->popupBG());
        this->msgbox->setLineColour(Aether::Colour{255, 255, 255, 0});
    }

    void Update::presentDownloading() {
        this->createNewMsgbox();

        // Add heading
        Aether::Element * body = new Aether::Element(0, 0, 600);
        Aether::Text * heading = new Aether::Text(50, 40, "Update.Downloading"_lang, headingFontSize);
        heading->setColour(this->app->theme()->FG());
        body->addElement(heading);
        body->setH(heading->h() + 50);

        // Add progress bar
        this->pbar = new Aether::RoundProgressBar(heading->x(), heading->y() + heading->h() + 25, body->w() - 100, 12);
        this->pbar->setBackgroundColour(this->app->theme()->muted2());
        this->pbar->setForegroundColour(this->app->theme()->accent());
        body->addElement(this->pbar);
        body->setH(body->h() + this->pbar->h() + 25);

        // Add statistics string
        this->statString = "0 KB / 0 KB (0%)";
        this->ptext = new Aether::Text(0, this->pbar->y() + this->pbar->h() + 15, this->statString, bodyFontSize);
        this->ptext->setColour(this->app->theme()->muted());
        this->ptext->setX(this->pbar->x() + (this->pbar->w() - this->ptext->w())/2);
        body->addElement(this->ptext);
        body->setH(body->h() + this->ptext->h() + 65);

        // Set body and show
        this->msgbox->setBodySize(body->w(), body->h());
        this->msgbox->setBody(body);
        this->app->addOverlay(this->msgbox);
    }

    void Update::presentInfo(const std::string & heading) {
        this->createNewMsgbox();

        // Add text and adjust size to match length of text
        Aether::Element * body = new Aether::Element();
        Aether::Text * text = new Aether::Text(100, 40, heading, headingFontSize);
        text->setColour(this->app->theme()->FG());
        body->setW(text->w() + 200);
        body->setH(text->h() + 80);
        text->setX((body->w() - text->w())/2);
        body->addElement(text);
        this->msgbox->setBodySize(body->w(), body->h());
        this->msgbox->setBody(body);
        this->app->addOverlay(this->msgbox);
    }

    void Update::presentInfo(const std::string & heading, const std::string & info, const std::function<void()> & callback) {
        this->createNewMsgbox();

        // Add OK callback
        this->msgbox->addTopButton("Common.OK"_lang, callback);
        this->msgbox->setLineColour(this->app->theme()->muted2());
        this->msgbox->setTextColour(this->app->theme()->accent());

        // Add heading
        Aether::Element * body = new Aether::Element(0, 0, 700);
        Aether::TextBlock * text = new Aether::TextBlock(50, 40, heading, headingFontSize, body->w() - 100);
        text->setColour(this->app->theme()->FG());
        body->addElement(text);
        body->setH(text->h() + 50);

        // Add info text block
        text = new Aether::TextBlock(50, text->y() + text->h() + 20, info, bodyFontSize, body->w() - 100);
        text->setColour(this->app->theme()->muted());
        body->addElement(text);
        body->setH(body->h() + text->h() + 70);
        this->msgbox->setBodySize(body->w(), body->h());
        this->msgbox->setBody(body);
        this->app->addOverlay(this->msgbox);
    }

    void Update::showChangelog() {
        // Fetch metadata regarding update
        Updater::Meta meta = this->updater->getMetadata();

        // Create a list, which contains the following text elements
        Aether::List * list = new Aether::List(this->container->x(), this->container->y(), this->container->w(), this->container->h() - 140, Aether::Padding::FitScrollbar);
        list->setShowScrollBar(true);
        list->setScrollBarColour(this->app->theme()->muted2());

        // Version heading
        Aether::Text * heading = new Aether::Text(0, 0, "", 24);
        list->addElement(heading);
        heading->setY(list->y());
        heading->setColour(this->app->theme()->FG());
        heading->setString(Utils::substituteTokens("Update.Available"_lang, meta.version.substr(1, meta.version.length() - 1)));
        list->addElement(new Aether::ListSeparator(30));

        // Changelog text block
        Aether::TextBlock * log = new Aether::TextBlock(0, 0, "", 18, 100);
        list->addElement(log);
        log->setColour(this->app->theme()->muted());
        log->setWrapWidth(log->w());
        log->setString(meta.changelog);
        this->container->addElement(list);

        // Update button (positioned underneath changelog)
        Aether::BorderButton * button = new Aether::BorderButton(this->container->x() + this->container->w()/2, list->y() + list->h(), 200, 60, 3, "Update.Update"_lang, 24, [this]() {
            // Create message box
            this->nextProgress = 0.0;
            this->presentDownloading();

            // Start a new thread for download
            this->operation = ThreadOperation::Download;
            this->threadDone = false;
            this->thread = std::async(std::launch::async, [this]() {
                bool ok = this->updater->downloadUpdate([this](long long dl, long long total) {
                    this->progressCallback(dl, total);
                });
                if (!ok) {
                    Utils::Fs::deleteFile(Path::App::UpdateFile);
                }
            });
        });
        button->setX(button->x() - button->w() - 30);
        int diff = this->container->y() + this->container->h() - button->y();
        button->setY(button->y() + (diff - button->h())/2);
        button->setBorderColour(this->app->theme()->FG());
        button->setTextColour(this->app->theme()->FG());
        this->container->addElement(button);

        // Cancel button (also positioned underneath, but next to update)
        button = new Aether::BorderButton(this->container->x() + this->container->w()/2, button->y(), button->w(), button->h(), 3, "Common.Cancel"_lang, 24, [this]() {
            this->app->popScreen();
        });
        button->setX(button->x() + 30);
        button->setBorderColour(this->app->theme()->FG());
        button->setTextColour(this->app->theme()->FG());
        this->container->addElement(button);
    }

    void Update::showError() {
        // Create heading
        Aether::Text * text = new Aether::Text(this->w()/2, this->container->y() + 170, "Update.Error.Check1"_lang, 24);
        text->setColour(this->app->theme()->FG());
        text->setX(text->x() - text->w()/2);
        this->container->addElement(text);

        // Create subheading
        text = new Aether::Text(this->w()/2, text->y() + 45, "Update.Error.Check2"_lang, 20);
        text->setColour(this->app->theme()->muted());
        text->setX(text->x() - text->w()/2);
        this->container->addElement(text);

        // Finally append current version
        text = new Aether::Text(this->w()/2, text->y() + text->h() + 10, Utils::substituteTokens("Update.Version"_lang, VER_STRING), 18);
        text->setColour(this->app->theme()->muted());
        text->setX(text->x() - text->w()/2);
        this->container->addElement(text);

        // Add back button
        Aether::BorderButton * button = new Aether::BorderButton(this->container->x() + this->container->w()/2 - 100, this->container->y() + this->container->h()*0.6, 200, 60, 3, "Common.Back"_lang, 24, [this]() {
            this->app->popScreen();
        });
        button->setBorderColour(this->app->theme()->FG());
        button->setTextColour(this->app->theme()->FG());
        this->container->addElement(button);
    }

    void Update::showUpToDate() {
        // Create heading
        Aether::Text * text = new Aether::Text(this->w()/2, this->container->y() + 170, "Update.UpToDate"_lang, 24);
        text->setColour(this->app->theme()->FG());
        text->setX(text->x() - text->w()/2);
        this->container->addElement(text);

        // And append current version
        text = new Aether::Text(this->w()/2, text->y() + text->h() + 10, Utils::substituteTokens("Update.Version"_lang, VER_STRING), 18);
        text->setColour(this->app->theme()->muted());
        text->setX(text->x() - text->w()/2);
        this->container->addElement(text);

        // Add back button
        Aether::BorderButton * button = new Aether::BorderButton(this->container->x() + this->container->w()/2 - 100, this->container->y() + this->container->h()*0.6, 200, 60, 3, "Common.Back"_lang, 24, [this]() {
            this->app->popScreen();
        });
        button->setBorderColour(this->app->theme()->FG());
        button->setTextColour(this->app->theme()->FG());
        this->container->addElement(button);
    }

    void Update::progressCallback(long long int dl, long long int total) {
        // While CURL is establishing a connection the total value changes and is small,
        // so we ignore that here to avoid flickering (size must be > 50 KB)
        if (total <= 50000) {
            return;
        }

        // Form statistics string
        this->nextProgress = 100.0 * (static_cast<double>(dl) / static_cast<double>(total));
        std::scoped_lock<std::mutex> mtx(this->mutex);
        this->statString = Utils::formatBytes(dl);
        this->statString += " / ";
        this->statString += Utils::formatBytes(total);
        this->statString += " (";
        this->statString += Utils::truncateToDecimalPlace(std::to_string(Utils::roundToDecimalPlace(this->nextProgress, 1)), 1);
        this->statString += "%)";
    }

    void Update::update(uint32_t dt) {
        Screen::update(dt);

        // Check thread status if it hasn't finished yet
        if (!this->threadDone) {
            if (this->thread.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                if (this->operation == ThreadOperation::CheckUpdate) {
                    // If the metadata about the update has a size, it must have been successful
                    Updater::Meta meta = this->updater->getMetadata();
                    if (meta.size > 0) {
                        // Up to date if strings match
                        if (meta.version == "v" VER_STRING) {
                            this->showUpToDate();
                        } else {
                            this->showChangelog();
                        }

                    } else {
                        this->showError();
                    }
                }
                this->msgbox->close();
                this->threadDone = true;

            // Otherwise if the thread isn't done and we're downloading, update the progress bar
            } else if (this->operation == ThreadOperation::Download) {
                if (this->nextProgress != this->pbar->value()) {
                    this->pbar->setValue(this->nextProgress);
                    std::scoped_lock<std::mutex> mtx(this->mutex);
                    this->ptext->setString(this->statString);
                    this->ptext->setX(this->pbar->x() + (this->pbar->w() - this->ptext->w())/2);
                }
            }

        // Otherwise if no thread is running, check if we need to start one
        } else {
            if (this->operation == ThreadOperation::Download) {
                // As the other thread deletes the file on an error, it obviously didn't succeed
                if (!Utils::Fs::fileExists(Path::App::UpdateFile)) {
                    this->presentInfo("Update.Error.Download1"_lang, "Update.Error.Download2"_lang, [this]() {
                        this->msgbox->close();
                    });
                    this->operation = ThreadOperation::None;

                // Otherwise extract it
                } else {
                    // Show message box
                    this->presentInfo("Update.Extracting"_lang);

                    // Start thread
                    this->operation = ThreadOperation::Extract;
                    this->threadDone = false;
                    this->thread = std::async(std::launch::async, [this]() {
                        // Extract zip, removing on success
                        if (Utils::Zip::extract(Path::App::UpdateFile, "/")) {
                            Utils::Fs::deleteFile(Path::App::UpdateFile);
                        }
                    });
                }

            } else if (this->operation == ThreadOperation::Extract) {
                // Once again if the file exists, it means an error occurred
                if (Utils::Fs::fileExists(Path::App::UpdateFile)) {
                    this->presentInfo("Update.Error.Extract1"_lang, "Update.Error.Extract2"_lang, [this]() {
                        Utils::Fs::deleteFile(Path::App::UpdateFile);
                        this->msgbox->close();
                    });

                // Otherwise show message box confirming success
                } else {
                    this->presentInfo("Update.Success1"_lang, "Update.Success2"_lang, [this]() {
                        this->msgbox->close();
                        this->app->sysmodule()->terminate();
                        this->app->exit(true);
                    });
                }
                this->operation = ThreadOperation::None;
            }
        }
    }

    void Update::onLoad() {
        // Background
        this->bg = new Aether::Image(0, 0, "romfs:/bg/main.png");
        this->addElement(this->bg);

        // Heading
        this->heading = new Aether::Text(60, 50, "Update.Heading"_lang, 50);
        this->heading->setColour(this->app->theme()->FG());
        this->addElement(this->heading);

        // Icon
        this->icon = new Aether::Image(this->w() - 200, 30, "romfs:/misc/noalbum.png");
        this->icon->setWH(150, 150);
        this->addElement(this->icon);

        // Container for elements which are added based on server response
        this->container = new Aether::Container(this->heading->x(), this->heading->y() + this->heading->h() + 20, this->w() - this->heading->x()*2);
        this->container->setH(this->h() - this->container->y());
        this->addElement(this->container);
        this->setFocussed(this->container);

        // Create initial "checking" message box
        this->msgbox = nullptr;
        this->presentInfo("Update.Checking"_lang);

        // Create updater object and start checking for update
        this->updater = new Updater();
        this->operation = ThreadOperation::CheckUpdate;
        this->threadDone = false;
        this->thread = std::async(std::launch::async, [this]() {
            bool ok = this->updater->checkForUpdate();
            if (ok && this->updater->availableUpdate()) {
                this->app->setHasUpdate(true);
            }
        });
    }

    void Update::onUnload() {
        this->thread.get();
        this->removeElement(this->bg);
        this->removeElement(this->icon);
        this->removeElement(this->container);
        this->removeElement(this->heading);
        delete this->msgbox;
        delete this->updater;
    }
};