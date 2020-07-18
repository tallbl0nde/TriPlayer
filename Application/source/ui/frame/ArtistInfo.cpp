#include "Application.hpp"
#include "ui/frame/ArtistInfo.hpp"
#include "ui/element/TextBox.hpp"
#include "utils/FS.hpp"

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
            this->metadata.name = this->name->string();
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
        this->imagePath->setString(this->metadata.imagePath.empty() ? "romfs:/misc/noartist.png" : this->metadata.imagePath);
        this->imagePath->setSelectable(false);
        this->imagePath->setTouchable(false);
        this->addElement(this->imagePath);

        // Image
        this->image = new Aether::Image(this->imagePath->x() + this->imagePath->w() + 50, this->imagePath->y(), (this->metadata.imagePath.empty() ? "romfs:/misc/noartist.png" : this->metadata.imagePath));
        this->image->setWH(this->w()*0.18, this->w()*0.18);
        this->addElement(this->image);

        // Fetch image (local)
        Aether::Container * c = new Aether::Container(txt->x(), this->imagePath->y() + this->imagePath->h() + 20, this->w()*0.62, 50);
        Aether::BorderButton * button = new Aether::BorderButton(c->x(), c->y(), this->w() * 0.3, 50, 2, "", 20, [this]() {
            // open file chooser or something
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
            if (this->imagePath->string().empty()) {
                return;
            }
            // 'Fake' remove for now
            this->deleteImage = true;
            this->removeElement(this->image);
            this->image = new Aether::Image(this->imagePath->x() + this->imagePath->w() + 50, this->imagePath->y(), "romfs:/misc/noartist.png");
            this->image->setWH(this->w()*0.18, this->w()*0.18);
            this->addElement(this->image);
            this->imagePath->setString("romfs:/misc/noartist.png");
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

        this->deleteImage = false;
        this->saveImage = false;
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
            this->updateAudioDBOverlay1();
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

    void ArtistInfo::updateAudioDBOverlay1() {
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

    void ArtistInfo::updateAudioDBOverlay2(const std::string & msg) {
        // Now remove old body and add OK button
        this->msgbox->emptyBody();
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

    void ArtistInfo::saveChanges() {
        // First delete old image if needed
        if (this->deleteImage) {
            Utils::Fs::deleteFile(this->metadata.imagePath);
            this->metadata.imagePath = "";
        }

        // Save new image
        if (this->saveImage) {
            Utils::Fs::writeFile(this->imagePath->string(), this->dlBuffer);
            this->metadata.imagePath = this->imagePath->string();
        }

        // Commit changes to db (this needs to be done a LOT better - it fails if sysmodule has database open)
        this->app->database()->close();
        this->app->database()->openReadWrite();
        bool ok = this->app->database()->updateArtist(this->metadata);
        this->app->database()->close();
        this->app->database()->openReadOnly();

        // Show message box indicating result
        this->oldmsgbox = this->msgbox;
        this->msgbox = new Aether::MessageBox();
        this->updateAudioDBOverlay2((ok ? "Your changes have been saved. You may need to restart the application for any changes to take complete effect." : "Unable to update the database, see the logs for more information!"));
    }

    void ArtistInfo::update(uint32_t dt) {
        Frame::update(dt);

        // Delete any old message boxes (needed as Aether::update has to be called on it before it can be deleted)
        if (this->oldmsgbox != nullptr) {
            delete this->oldmsgbox;
            this->oldmsgbox = nullptr;
        }

        if (this->threadRunning) {
            // Once the thread is done, update the msgbox to indicate the result
            if (this->dlThread.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                switch (this->dlThread.get()) {
                    case Metadata::DownloadResult::Error:
                        this->updateAudioDBOverlay2("An unknown error occurred during downloading. This could be due to:\n- A poor internet connection\n- The server is overloaded\n- An issue with this application\n\nPlease try again later.");
                        break;

                    case Metadata::DownloadResult::NotFound:
                        this->updateAudioDBOverlay2("An artist could not be found with the name '" + this->metadata.name + "'. Please check your spelling or set an image manually.");
                        break;

                    case Metadata::DownloadResult::NoImage:
                        this->updateAudioDBOverlay2("'" + this->metadata.name + "' was found on TheAudioDB but has no image associated with them.");
                        break;

                    case Metadata::DownloadResult::Success:
                        this->msgbox->close();
                        // Remove old image and replace with new one
                        this->removeElement(this->image);
                        this->image = new Aether::Image(this->imagePath->x() + this->imagePath->w() + 50, this->imagePath->y(), &this->dlBuffer[0], this->dlBuffer.size());
                        this->image->setWH(this->w()*0.18, this->w()*0.18);
                        this->addElement(this->image);

                        // Update new image path
                        this->saveImage = true;
                        this->imagePath->setString("/switch/TriPlayer/images/artist/" + std::to_string(this->metadata.tadbID) + ".png");
                        break;
                }
                this->threadRunning = false;
            }
        }
    }

    ArtistInfo::~ArtistInfo() {
        if (this->msgbox) {
            this->msgbox->close();
        }
        delete this->oldmsgbox;
        delete this->msgbox;
    }
};