#include "Application.hpp"
#include "ui/frame/ArtistInfo.hpp"
#include "ui/element/TextBox.hpp"
#include "utils/FS.hpp"
#include "utils/Utils.hpp"

// Location of default image
#define DEFAULT_IMAGE "romfs:/misc/noartist.png"
// Default path for file browser
#define FILE_BROWSER_ROOT "/"
// Accepted image extensions
static const std::vector<std::string> FILE_EXTENSIONS = {".jpg", ".jpeg", ".jfif", ".png", ".JPG", ".JPEG", ".JFIF", ".PNG"};
// Save location of images
#define SAVE_LOCATION "/switch/TriPlayer/images/artist/"

// This whole file/frame is a mess behind the scenes :P
namespace Frame {
    ArtistInfo::ArtistInfo(Main::Application * a, ArtistID id) : Frame(a) {
        // Remove list + headings (I should redo Frame to avoid this)
        this->removeElement(this->list);
        this->removeElement(this->titleH);
        this->removeElement(this->artistH);
        this->removeElement(this->albumH);
        this->removeElement(this->lengthH);

        // Get artist metadata from ID
        this->metadata = this->app->database()->getArtistMetadataForID(id);
        if (this->metadata.ID < 0) {
            // Error message
            Aether::Text * t = new Aether::Text(this->x() + this->w()/2, this->y() + this->h()/2, "An error occurred fetching the data. Please restart the app and try again.", 20);
            t->setX(t->x() - t->w()/2);
            t->setY(t->y() - t->h()/2);
            t->setColour(this->app->theme()->FG());
            this->addElement(t);
            return;
        }

        // Create and position all elements next
        this->heading->setString("Artist Information");

        // Name
        Aether::Text * txt = new Aether::Text(this->heading->x(), this->heading->y() + this->heading->h() + 20, "Name", 30);
        txt->setColour(this->app->theme()->FG());
        this->addElement(txt);
        this->name = new CustomElm::TextBox(txt->x(), txt->y() + txt->h() + 10, this->w() * 0.62, 50);
        this->name->setBoxColour(this->app->theme()->muted2());
        this->name->setTextColour(this->app->theme()->FG());
        this->name->setKeyboardHint("Name");
        this->name->setKeyboardLimit(100);
        this->name->setString(this->metadata.name);
        this->name->setKeyboardCallback([this]() {
            std::string str = this->name->string();

            // Remove trailing spaces
            while (!str.empty() && str[str.length() - 1] == ' ') {
                str.erase(str.length() - 1);
            }

            this->metadata.name = str;
            this->name->setString(str);
        });
        this->addElement(this->name);

        // Database ID
        txt = new Aether::Text(this->name->x() + this->name->w() + 50, txt->y(), "Database ID", 30);
        txt->setColour(this->app->theme()->FG());
        this->addElement(txt);
        CustomElm::TextBox * box = new CustomElm::TextBox(txt->x(), txt->y() + txt->h() + 10, this->w() * 0.18, 50);
        box->setBoxColour(this->app->theme()->muted2());
        box->setTextColour(this->app->theme()->muted());
        box->setString(std::to_string(this->metadata.ID));
        box->setSelectable(false);
        box->setTouchable(false);
        this->addElement(box);

        // Image heading
        txt = new Aether::Text(this->heading->x(), box->y() + box->h() + 20, "Image", 30);
        txt->setColour(this->app->theme()->FG());
        this->addElement(txt);

        // Image path
        this->imagePath = new CustomElm::TextBox(txt->x(), txt->y() + txt->h() + 10, this->w() * 0.62, 50);
        this->imagePath->setBoxColour(this->app->theme()->muted2());
        this->imagePath->setTextColour(this->app->theme()->muted());
        this->imagePath->setString(this->metadata.imagePath.empty() ? DEFAULT_IMAGE : this->metadata.imagePath);
        this->imagePath->setSelectable(false);
        this->imagePath->setTouchable(false);
        this->addElement(this->imagePath);

        // Image
        this->image = new Aether::Image(this->imagePath->x() + this->imagePath->w() + 50, this->imagePath->y(), (this->metadata.imagePath.empty() ? DEFAULT_IMAGE : this->metadata.imagePath));
        this->image->setWH(this->w()*0.18, this->w()*0.18);
        this->addElement(this->image);

        // Fetch image (local)
        Aether::Container * c = new Aether::Container(txt->x(), this->imagePath->y() + this->imagePath->h() + 20, this->w()*0.62, 50);
        Aether::BorderButton * button = new Aether::BorderButton(c->x(), c->y(), this->w() * 0.3, 50, 2, "", 20, [this]() {
            this->createFileBrowser();
        });
        button->setString("Replace from SD Card");
        button->setBorderColour(this->app->theme()->FG());
        button->setTextColour(this->app->theme()->FG());
        c->addElement(button);

        // Fetch image (TheAudioDB)
        button = new Aether::BorderButton(button->x() + button->w() + this->w()*0.02, button->y(), this->w() * 0.3, 50, 2, "", 20, [this]() {
            this->createAudioDBOverlay();
        });
        button->setString("Replace from TheAudioDB");
        button->setBorderColour(this->app->theme()->FG());
        button->setTextColour(this->app->theme()->FG());
        c->addElement(button);
        this->addElement(c);

        // Remove image
        button = new Aether::BorderButton(txt->x() + this->w()*0.16, button->y() + button->h() + 20, this->w() * 0.3, 50, 2, "", 20, [this]() {
            this->removeImage();
        });
        button->setString("Remove Image");
        button->setBorderColour(this->app->theme()->FG());
        button->setTextColour(this->app->theme()->FG());
        this->addElement(button);

        // Save button
        Aether::FilledButton * save = new Aether::FilledButton(button->x() + button->w()/2 - 75, button->y() + button->h() + 25, 150, 50, "Save", 26, [this]() {
            this->saveChanges();
        });
        save->setFillColour(this->app->theme()->accent());
        save->setTextColour(Aether::Colour{0, 0, 0, 255});
        this->addElement(save);

        this->checkFB = false;
        this->browser = nullptr;
        this->updateImage = false;
        this->oldmsgbox = nullptr;
        this->msgbox = nullptr;
        this->threadRunning = false;
    }

    void ArtistInfo::createAudioDBOverlay() {
        delete this->msgbox;

        // Create message box
        this->msgbox = new Aether::MessageBox();
        this->msgbox->addLeftButton("Cancel", [this]() {
            this->msgbox->close();
        });
        this->msgbox->addRightButton("OK", [this]() {
            this->updateAudioDBOverlay();
            this->dlBuffer.clear();
            this->dlThread = std::async(std::launch::async, [this]() -> Metadata::DownloadResult {
                return Metadata::downloadArtistImage(this->metadata.name, this->dlBuffer, this->metadata.tadbID);
            });
            this->threadRunning = true;
        });
        this->msgbox->setLineColour(this->app->theme()->muted2());
        this->msgbox->setRectangleColour(this->app->theme()->popupBG());
        this->msgbox->setTextColour(this->app->theme()->accent());

        // Create tip text and set as body
        Aether::Element * body = new Aether::Element(0, 0, 700);
        Aether::TextBlock * tips = new Aether::TextBlock(40, 40, "", 24, 620);
        tips->setString("An artist image will be searched for and downloaded using TheAudioDB.\n\nFor best results:\n- Ensure the artist's name is correct\n- Make sure you have a stable internet connection\n\nYou will be unable to control playback while the image is downloading.");
        tips->setColour(this->app->theme()->FG());
        body->addElement(tips);
        body->setH(tips->h() + 80);
        this->msgbox->setBody(body);
        this->msgbox->setBodySize(body->w(), body->h());

        this->app->addOverlay(this->msgbox);
    }

    void ArtistInfo::updateAudioDBOverlay() {
        // Remove old msgbox
        this->msgbox->close();
        this->oldmsgbox = this->msgbox;

        // Create one without buttons
        this->msgbox = new Aether::MessageBox();
        this->msgbox->onButtonPress(Aether::Button::B, nullptr);
        this->msgbox->setLineColour(Aether::Colour{0, 0, 0, 0});
        this->msgbox->setRectangleColour(this->app->theme()->popupBG());
        Aether::Element * body = new Aether::Element(0, 0, 700);
        Aether::TextBlock * tips = new Aether::TextBlock(40, 40, "", 24, 620);
        tips->setString("Searching for '" + this->metadata.name +"' on TheAudioDB.com...");
        tips->setColour(this->app->theme()->FG());
        body->addElement(tips);
        body->setH(tips->h() + 80);
        this->msgbox->setBody(body);
        this->msgbox->setBodySize(body->w(), body->h());
        this->app->addOverlay(this->msgbox);
    }

    void ArtistInfo::createInfoOverlay(const std::string & msg) {
        // Remove old msgbox
        if (this->msgbox != nullptr) {
            this->msgbox->close();
        }
        this->oldmsgbox = this->msgbox;

        // Now create new one
        this->msgbox = new Aether::MessageBox();
        this->msgbox->addTopButton("OK", [this]() {
            this->msgbox->close();
        });
        this->msgbox->setLineColour(this->app->theme()->muted2());
        this->msgbox->setRectangleColour(this->app->theme()->popupBG());
        this->msgbox->setTextColour(this->app->theme()->accent());
        Aether::Element * body = new Aether::Element(0, 0, 700);
        Aether::TextBlock * tips = new Aether::TextBlock(40, 40, "", 24, 620);
        tips->setString(msg);
        tips->setColour(this->app->theme()->FG());
        body->addElement(tips);
        body->setH(tips->h() + 80);
        this->msgbox->setBody(body);
        this->msgbox->setBodySize(body->w(), body->h());
        this->app->addOverlay(this->msgbox);
    }

    void ArtistInfo::createFileBrowser() {
        // Create if doesn't exist
        if (this->browser == nullptr) {
            this->browser = new CustomOvl::FileBrowser(700, 600);
            this->browser->setAccentColour(this->app->theme()->accent());
            this->browser->setMutedLineColour(this->app->theme()->muted2());
            this->browser->setMutedTextColour(this->app->theme()->muted());
            this->browser->setRectangleColour(this->app->theme()->popupBG());
            this->browser->setTextColour(this->app->theme()->FG());
            this->browser->setCancelText("Cancel");
            this->browser->setHeadingText("Select an Image");
            this->browser->setExtensions(FILE_EXTENSIONS);
        }

        // Set path and show
        this->checkFB = true;
        this->browser->resetFile();
        this->browser->setPath(FILE_BROWSER_ROOT);
        this->app->addOverlay(this->browser);
    }

    void ArtistInfo::saveChanges() {
        // Don't permit blank name
        if (this->metadata.name.empty()) {
            this->createInfoOverlay("You can't set a blank name. Nice try ;)");
            return;
        }

        // Set image path if needed
        if (this->updateImage) {
            if (!this->metadata.imagePath.empty()) {
                this->metadata.imagePath = "";
            }
            if (!this->newImagePath.empty() || !this->dlBuffer.empty()) {
                this->metadata.imagePath = this->imagePath->string();
            }
        }

        // Commit changes to db (acquires lock and then writes)
        this->app->database()->close();
        this->app->sysmodule()->waitRequestDBLock();
        this->app->database()->openReadWrite();
        bool ok = this->app->database()->updateArtist(this->metadata);
        this->app->database()->close();
        this->app->sysmodule()->sendReleaseDBLock();
        this->app->database()->openReadOnly();

        // If updated ok actually manipulate image file(s)
        if (ok && this->updateImage) {
            // First delete old image if needed
            if (!this->metadata.imagePath.empty()) {
                Utils::Fs::deleteFile(this->metadata.imagePath);
            }

            // Save new image if there is one
            if (!this->newImagePath.empty()) {
                Utils::Fs::copyFile(this->newImagePath, this->imagePath->string());
            } else if (!this->dlBuffer.empty()) {
                Utils::Fs::writeFile(this->imagePath->string(), this->dlBuffer);
            }
        }

        // Show message box indicating result
        this->createInfoOverlay((ok ? "Your changes have been saved. You may need to restart the application for any changes to take complete effect." : "Unable to update the database, see the logs for more information!"));
    }

    void ArtistInfo::update(uint32_t dt) {
        Frame::update(dt);

        // Delete any old message boxes (needed as Aether::update has to be called on it before it can be deleted)
        if (this->oldmsgbox != nullptr) {
            delete this->oldmsgbox;
            this->oldmsgbox = nullptr;
        }

        // Once the thread is done, update the msgbox to indicate the result
        if (this->threadRunning) {
            if (this->dlThread.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                switch (this->dlThread.get()) {
                    case Metadata::DownloadResult::Error:
                        this->createInfoOverlay("An unknown error occurred during downloading. This could be due to:\n- A poor internet connection\n- The server is overloaded\n- An issue with this application\n\nPlease try again later.");
                        break;

                    case Metadata::DownloadResult::NotFound:
                        this->createInfoOverlay("An artist could not be found with the name '" + this->metadata.name + "'. Please check your spelling or set an image manually.");
                        break;

                    case Metadata::DownloadResult::NoImage:
                        this->createInfoOverlay("'" + this->metadata.name + "' was found on TheAudioDB but has no image associated with them.");
                        break;

                    case Metadata::DownloadResult::Success:
                        this->msgbox->close();
                        this->updateImageFromDL();
                        break;
                }
                this->threadRunning = false;
            }
        }

        // Check the file browser result and create popup/image
        if (this->checkFB) {
            if (this->browser->shouldClose()) {
                std::string path = this->browser->chosenFile();
                if (!path.empty()) {
                    this->updateImageFromPath(path);
                }
                this->checkFB = false;
            }
        }
    }

    void ArtistInfo::removeImage() {
        // 'Fake' remove
        this->updateImage = true;
        this->dlBuffer.clear();
        this->newImagePath.clear();
        this->removeElement(this->image);
        this->image = new Aether::Image(this->imagePath->x() + this->imagePath->w() + 50, this->imagePath->y(), DEFAULT_IMAGE);
        this->image->setWH(this->w()*0.18, this->w()*0.18);
        this->addElement(this->image);
        this->imagePath->setString(DEFAULT_IMAGE);
    }

    void ArtistInfo::updateImageFromDL() {
        // Remove old image and replace with new one
        this->removeElement(this->image);
        this->image = new Aether::Image(this->imagePath->x() + this->imagePath->w() + 50, this->imagePath->y(), &this->dlBuffer[0], this->dlBuffer.size());
        this->image->setWH(this->w()*0.18, this->w()*0.18);
        this->addElement(this->image);

        // Update new image path (everything just has extension .png)
        this->updateImage = true;
        this->newImagePath.clear();
        this->imagePath->setString(SAVE_LOCATION + std::to_string(this->metadata.tadbID) + ".png");
    }

    void ArtistInfo::updateImageFromPath(const std::string & path) {
        // Attempt to create new image
        Aether::Image * tmpImage = new Aether::Image(this->imagePath->x() + this->imagePath->w() + 50, this->imagePath->y(), path);
        tmpImage->setWH(this->w()*0.18, this->w()*0.18);

        // Show error if image wasn't created
        if (tmpImage->texW() == 0 || tmpImage->texH() == 0) {
            this->createInfoOverlay("An error occurred reading the selected image. This may be due to a corrupted image or incorrect file extension.");
            delete tmpImage;

        // Otherwise replace image
        } else {
            this->removeElement(this->image);
            this->image = tmpImage;
            this->addElement(this->image);
            this->updateImage = true;

            // Generate unique path (if you're really unlucky this could run indefinitely - but with 62^10 combinations we should be right :P)
            std::string ext = Utils::Fs::getExtension(path);
            std::string rand;
            do {
                rand = Utils::randomString(10);
            } while (Utils::Fs::fileExists(SAVE_LOCATION + rand + ext));
            this->imagePath->setString(SAVE_LOCATION + rand + ext);
            this->newImagePath = path;
            this->dlBuffer.clear();
        }
    }

    ArtistInfo::~ArtistInfo() {
        if (this->msgbox) {
            this->msgbox->close();
        }
        delete this->oldmsgbox;
        delete this->msgbox;
        delete this->browser;
    }
};