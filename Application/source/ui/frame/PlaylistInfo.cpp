#include "Application.hpp"
#include "Paths.hpp"
#include "ui/element/TextBox.hpp"
#include "ui/frame/PlaylistInfo.hpp"
#include "ui/overlay/FileBrowser.hpp"
#include "utils/FS.hpp"
#include "utils/Image.hpp"
#include "utils/MP3.hpp"
#include "utils/Utils.hpp"

// Default path for file browser
#define FILE_BROWSER_ROOT "/"
// Accepted image extensions
static const std::vector<std::string> FILE_EXTENSIONS_AUD = {".mp3", ".MP3"};
static const std::vector<std::string> FILE_EXTENSIONS_IMG = {".jpg", ".jpeg", ".jfif", ".png", ".JPG", ".JPEG", ".JFIF", ".PNG"};

// This whole file/frame is a mess behind the scenes :P
namespace Frame {
    PlaylistInfo::PlaylistInfo(Main::Application * a, AlbumID id) : Frame(a) {
        // Remove list + headings (I should redo Frame to avoid this)
        this->removeElement(this->list);
        this->removeElement(this->titleH);
        this->removeElement(this->artistH);
        this->removeElement(this->albumH);
        this->removeElement(this->lengthH);

        // Get album metadata from ID
        this->metadata = this->app->database()->getPlaylistMetadataForID(id);
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
        this->heading->setString("Playlist Information");

        // Name
        Aether::Text * txt = new Aether::Text(this->heading->x(), this->heading->y() + this->heading->h() + 20, "Name", 30);
        txt->setColour(this->app->theme()->FG());
        this->addElement(txt);
        this->name = new CustomElm::TextBox(txt->x(), txt->y() + txt->h() + 10, this->w() * 0.62, 50);
        this->name->setBoxColour(this->app->theme()->muted2());
        this->name->setTextColour(this->app->theme()->FG());
        this->name->setKeyboardHint("Name");
        this->name->setKeyboardLimit(50);
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
        this->imagePath->setString(this->metadata.imagePath.empty() ? Path::App::DefaultArtistFile : this->metadata.imagePath);
        this->imagePath->setSelectable(false);
        this->imagePath->setTouchable(false);
        this->addElement(this->imagePath);

        // Image
        this->image = new Aether::Image(this->imagePath->x() + this->imagePath->w() + 50, this->imagePath->y(), (this->metadata.imagePath.empty() ? Path::App::DefaultArtistFile : this->metadata.imagePath));
        this->image->setWH(this->w()*0.18, this->w()*0.18);
        this->addElement(this->image);

        // Fetch image (id3 tags)
        Aether::BorderButton * button = new Aether::BorderButton(txt->x(), this->imagePath->y() + this->imagePath->h() + 20, this->w() * 0.3, 50, 2, "", 20, [this]() {
            this->createFileBrowser(FBType::Audio);
        });
        button->setString("Replace from Audio File");
        button->setBorderColour(this->app->theme()->FG());
        button->setTextColour(this->app->theme()->FG());
        this->addElement(button);

        // Fetch image (local image)
        button = new Aether::BorderButton(button->x() + button->w() + this->w()*0.02, button->y(), this->w() * 0.3, 50, 2, "", 20, [this]() {
            this->createFileBrowser(FBType::Image);
        });
        button->setString("Replace from SD Card");
        button->setBorderColour(this->app->theme()->FG());
        button->setTextColour(this->app->theme()->FG());
        this->addElement(button);

        // Remove image
        button = new Aether::BorderButton(button->x() - this->w()*0.16, button->y() + button->h() + 20, this->w() * 0.3, 50, 2, "", 20, [this]() {
            this->removeImage();
        });
        button->setString("Remove Image");
        button->setBorderColour(this->app->theme()->FG());
        button->setTextColour(this->app->theme()->FG());
        this->addElement(button);

        // Save button
        this->saveButton = new Aether::FilledButton(button->x() + button->w()/2 - 75, button->y() + button->h() + 25, 150, 50, "Save", 26, [this]() {
            this->saveChanges();
        });
        this->saveButton->setFillColour(this->app->theme()->accent());
        this->saveButton->setTextColour(Aether::Colour{0, 0, 0, 255});
        this->addElement(this->saveButton);

        this->checkFB = false;
        this->browser = nullptr;
        this->updateImage = false;
        this->oldmsgbox = nullptr;
        this->msgbox = nullptr;
    }

    void PlaylistInfo::createInfoOverlay(const std::string & msg) {
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

    void PlaylistInfo::createFileBrowser(FBType t) {
        this->fileType = t;

        delete this->browser;
        this->browser = new CustomOvl::FileBrowser(700, 600);
        this->browser->setAccentColour(this->app->theme()->accent());
        this->browser->setMutedLineColour(this->app->theme()->muted2());
        this->browser->setMutedTextColour(this->app->theme()->muted());
        this->browser->setRectangleColour(this->app->theme()->popupBG());
        this->browser->setTextColour(this->app->theme()->FG());
        this->browser->setCancelText("Cancel");
        switch (t) {
            case FBType::Audio:
                this->browser->setHeadingText("Select an Audio file");
                this->browser->setExtensions(FILE_EXTENSIONS_AUD);
                break;

            case FBType::Image:
                this->browser->setHeadingText("Select an Image");
                this->browser->setExtensions(FILE_EXTENSIONS_IMG);
                break;
        }

        // Set path and show
        this->checkFB = true;
        this->browser->resetFile();
        this->browser->setPath(FILE_BROWSER_ROOT);
        this->app->addOverlay(this->browser);
    }

    void PlaylistInfo::saveChanges() {
        // Don't permit blank name
        if (this->metadata.name.empty()) {
            this->createInfoOverlay("You can't set a blank name. Nice try ;)");
            return;
        }

        // Set image path if needed
        std::string oldPath = this->metadata.imagePath;
        if (this->updateImage) {
            if (!this->metadata.imagePath.empty()) {
                this->metadata.imagePath = "";
            }
            if (!this->newImagePath.empty() || !this->dlBuffer.empty()) {
                this->metadata.imagePath = this->imagePath->string();
            }
        }

        // Copy new image to disk
        if (this->updateImage && !this->metadata.imagePath.empty()) {
            // Extract image (if needed) and resize
            if (!this->newImagePath.empty()) {
                Utils::Fs::readFile(this->newImagePath, this->dlBuffer);
            }
            bool resized = Utils::Image::resize(this->dlBuffer, 400, 400);
            if (!resized) {
                Log::writeWarning("[META] Couldn't resize playlist image, saving with original dimensions");
            }

            // Write (hopefully resized) to file
            Utils::Fs::writeFile(this->metadata.imagePath, this->dlBuffer);
        }

        // Commit changes to db (acquires lock and then writes)
        this->app->lockDatabase();
        bool ok = this->app->database()->updatePlaylist(this->metadata);
        this->app->unlockDatabase();

        // Delete image if everything succeeded, revert copied image on an error
        if (this->updateImage) {
            if (ok && !oldPath.empty()) {
                Utils::Fs::deleteFile(oldPath);

            } else if (!ok && !this->metadata.imagePath.empty()) {
                Utils::Fs::deleteFile(this->metadata.imagePath);
            }
        }

        // Show message box indicating result
        this->createInfoOverlay((ok ? "Your changes have been saved. You may need to restart the application for any changes to take complete effect." : "Unable to update the database, see the logs for more information!"));
    }

    void PlaylistInfo::update(uint32_t dt) {
        Frame::update(dt);

        // Delete any old message boxes (needed as Aether::update has to be called on it before it can be deleted)
        if (this->oldmsgbox != nullptr) {
            delete this->oldmsgbox;
            this->oldmsgbox = nullptr;
        }

        // Check the file browser result and create popup/image
        if (this->checkFB) {
            if (this->browser->shouldClose()) {
                std::string path = this->browser->chosenFile();
                if (!path.empty()) {
                    switch (this->fileType) {
                        case FBType::Audio:
                            this->updateImageFromID3(path);
                            break;

                        case FBType::Image:
                            this->updateImageFromPath(path);
                            break;
                    }
                }
                this->checkFB = false;
            }
        }
    }

    void PlaylistInfo::removeImage() {
        // 'Fake' remove
        this->updateImage = true;
        this->dlBuffer.clear();
        this->newImagePath.clear();
        this->removeElement(this->image);
        this->image = new Aether::Image(this->imagePath->x() + this->imagePath->w() + 50, this->imagePath->y(), Path::App::DefaultArtistFile);
        this->image->setWH(this->w()*0.18, this->w()*0.18);
        this->addElement(this->image);
        this->imagePath->setString(Path::App::DefaultArtistFile);
    }

    void PlaylistInfo::updateImageFromID3(const std::string & path) {
        // Extract image
        this->dlBuffer = Utils::MP3::getArtFromID3(path);
        if (this->dlBuffer.empty()) {
            this->createInfoOverlay("No album art was found in the selected file.");
            return;
        }

        // Attempt to create new image
        Aether::Image * tmpImage = new Aether::Image(this->imagePath->x() + this->imagePath->w() + 50, this->imagePath->y(), &this->dlBuffer[0], this->dlBuffer.size());
        tmpImage->setWH(this->w()*0.18, this->w()*0.18);

        // Show error if image wasn't created
        if (tmpImage->texW() == 0 || tmpImage->texH() == 0) {
            this->createInfoOverlay("An error occurred processing the file. This may mean the file is corrupt or the stored album art is not supported.");
            delete tmpImage;

        // Otherwise replace image
        } else {
            this->removeElement(this->image);
            this->image = tmpImage;
            this->addElement(this->image);
            this->updateImage = true;

            // Generate unique path (if you're really unlucky this could run indefinitely - but with 62^10 combinations we should be right :P)
            std::string rand;
            do {
                rand = Utils::randomString(10);
            } while (Utils::Fs::fileExists(Path::App::PlaylistImageFolder + rand + ".png"));
            this->newImagePath.clear();
            this->imagePath->setString(Path::App::PlaylistImageFolder + rand + ".png");
        }
    }

    void PlaylistInfo::updateImageFromPath(const std::string & path) {
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
            } while (Utils::Fs::fileExists(Path::App::PlaylistImageFolder + rand + ext));
            this->imagePath->setString(Path::App::PlaylistImageFolder + rand + ext);
            this->newImagePath = path;
            this->dlBuffer.clear();
        }
    }

    void PlaylistInfo::updateColours() {
        this->saveButton->setFillColour(this->app->theme()->accent());
    }

    PlaylistInfo::~PlaylistInfo() {
        if (this->msgbox) {
            this->msgbox->close();
        }
        delete this->oldmsgbox;
        delete this->msgbox;
        delete this->browser;
    }
};