#include "Application.hpp"
#include "Paths.hpp"
#include "ui/frame/SongInfo.hpp"
#include "ui/element/TextBox.hpp"
#include "utils/FS.hpp"
#include "utils/Utils.hpp"

// This whole file/frame is a mess behind the scenes :P
namespace Frame {
    SongInfo::SongInfo(Main::Application * a, SongID id) : Frame(a) {
        // Remove list + headings (I should redo Frame to avoid this)
        this->bottomContainer->removeElement(this->list);
        this->topContainer->removeElement(this->titleH);
        this->topContainer->removeElement(this->artistH);
        this->topContainer->removeElement(this->albumH);
        this->topContainer->removeElement(this->lengthH);
        this->sort->setHidden(true);
        this->topContainer->setHasSelectable(false);

        // Get artist metadata from ID
        this->metadata = this->app->database()->getSongMetadataForID(id);
        if (this->metadata.ID < 0) {
            // Error message
            Aether::Text * t = new Aether::Text(this->x() + this->w()/2, this->y() + this->h()/2, "Common.Error.Database"_lang, 20);
            t->setX(t->x() - t->w()/2);
            t->setY(t->y() - t->h()/2);
            t->setColour(this->app->theme()->FG());
            this->bottomContainer->addElement(t);
            return;
        }

        // Create and position all elements next
        this->heading->setString("Song.Information.Heading"_lang);

        // Title
        Aether::Text * txt = new Aether::Text(this->heading->x(), this->heading->y() + this->heading->h() + 20, "Common.Title"_lang, 30);
        txt->setColour(this->app->theme()->FG());
        this->bottomContainer->addElement(txt);
        this->title = new CustomElm::TextBox(txt->x(), txt->y() + txt->h() + 10, this->w() * 0.62, 50);
        this->title->setBoxColour(this->app->theme()->muted2());
        this->title->setTextColour(this->app->theme()->FG());
        this->title->setKeyboardHint("Common.Title"_lang);
        this->title->setKeyboardLimit(100);
        this->title->setString(this->metadata.title);
        this->title->setKeyboardCallback([this]() {
            std::string str = this->title->string();

            // Remove trailing spaces
            while (!str.empty() && str[str.length() - 1] == ' ') {
                str.erase(str.length() - 1);
            }

            this->metadata.title = str;
            this->title->setString(str);
        });
        this->bottomContainer->addElement(this->title);

        // Database ID
        txt = new Aether::Text(this->title->x() + this->title->w() + 50, txt->y(), "Common.DatabaseID"_lang, 30);
        txt->setColour(this->app->theme()->FG());
        this->bottomContainer->addElement(txt);
        CustomElm::TextBox * box = new CustomElm::TextBox(txt->x(), txt->y() + txt->h() + 10, this->w() * 0.18, 50);
        box->setBoxColour(this->app->theme()->muted2());
        box->setTextColour(this->app->theme()->muted());
        box->setString(std::to_string(this->metadata.ID));
        box->setSelectable(false);
        box->setTouchable(false);
        this->bottomContainer->addElement(box);

        // Artist
        txt = new Aether::Text(this->heading->x(), box->y() + box->h() + 15, "Artist.Artist"_lang, 30);
        txt->setColour(this->app->theme()->FG());
        this->bottomContainer->addElement(txt);
        this->artist = new CustomElm::TextBox(txt->x(), txt->y() + txt->h() + 10, this->w() * 0.62, 50);
        this->artist->setBoxColour(this->app->theme()->muted2());
        this->artist->setTextColour(this->app->theme()->FG());
        this->artist->setKeyboardHint("Artist.Artist"_lang);
        this->artist->setKeyboardLimit(100);
        this->artist->setString(this->metadata.artist);
        this->artist->setKeyboardCallback([this]() {
            std::string str = this->artist->string();

            // Remove trailing spaces
            while (!str.empty() && str[str.length() - 1] == ' ') {
                str.erase(str.length() - 1);
            }

            this->metadata.artist = str;
            this->artist->setString(str);
        });
        this->bottomContainer->addElement(this->artist);

        // Track Number
        txt = new Aether::Text(this->artist->x() + this->artist->w() + 50, txt->y(), "Song.Information.TrackNumber"_lang, 30);
        txt->setColour(this->app->theme()->FG());
        this->bottomContainer->addElement(txt);
        this->trackNumber = new CustomElm::NumberBox(box->x(), this->artist->y(), box->w(), this->artist->h());
        this->trackNumber->setBoxColour(this->app->theme()->muted2());
        this->trackNumber->setTextColour(this->app->theme()->FG());
        this->trackNumber->setValue(this->metadata.trackNumber);
        this->trackNumber->setNumpadAllowFloats(false);
        this->trackNumber->setNumpadHint("Song.Information.TrackNumber"_lang);
        this->trackNumber->setNumpadLimit(4);
        this->trackNumber->setNumpadNegative(false);
        this->trackNumber->setNumpadCallback([this]() {
            this->metadata.trackNumber = this->trackNumber->value();
        });
        this->bottomContainer->addElement(this->trackNumber);

        // Album
        txt = new Aether::Text(this->heading->x(), this->artist->y() + this->artist->h() + 15, "Album.Album"_lang, 30);
        txt->setColour(this->app->theme()->FG());
        this->bottomContainer->addElement(txt);
        this->album = new CustomElm::TextBox(txt->x(), txt->y() + txt->h() + 10, this->w() * 0.62, 50);
        this->album->setBoxColour(this->app->theme()->muted2());
        this->album->setTextColour(this->app->theme()->FG());
        this->album->setKeyboardHint("Album.Album"_lang);
        this->album->setKeyboardLimit(100);
        this->album->setString(this->metadata.album);
        this->album->setKeyboardCallback([this]() {
            std::string str = this->album->string();

            // Remove trailing spaces
            while (!str.empty() && str[str.length() - 1] == ' ') {
                str.erase(str.length() - 1);
            }

            this->metadata.album = str;
            this->album->setString(str);
        });
        this->bottomContainer->addElement(this->album);

        // Disc Number
        txt = new Aether::Text(this->album->x() + this->album->w() + 50, txt->y(), "Song.Information.DiscNumber"_lang, 30);
        txt->setColour(this->app->theme()->FG());
        this->bottomContainer->addElement(txt);
        this->discNumber = new CustomElm::NumberBox(this->trackNumber->x(), this->album->y(), this->trackNumber->w(), this->album->h());
        this->discNumber->setBoxColour(this->app->theme()->muted2());
        this->discNumber->setTextColour(this->app->theme()->FG());
        this->discNumber->setValue(this->metadata.discNumber);
        this->discNumber->setNumpadAllowFloats(false);
        this->discNumber->setNumpadHint("Song.Information.DiscNumber"_lang);
        this->discNumber->setNumpadLimit(4);
        this->discNumber->setNumpadNegative(false);
        this->discNumber->setNumpadCallback([this]() {
            this->metadata.discNumber = this->discNumber->value();
        });
        this->bottomContainer->addElement(this->discNumber);

        // File path
        txt = new Aether::Text(this->heading->x(), this->album->y() + this->album->h() + 15, "Song.Information.FileLocation"_lang, 30);
        txt->setColour(this->app->theme()->FG());
        this->bottomContainer->addElement(txt);
        this->filePath = new CustomElm::TextBox(txt->x(), txt->y() + txt->h() + 10, this->w() * 0.62, 50);
        this->filePath->setBoxColour(this->app->theme()->muted2());
        this->filePath->setTextColour(this->app->theme()->muted());
        this->filePath->setString(this->metadata.path);
        this->filePath->setSelectable(false);
        this->filePath->setTouchable(false);
        this->bottomContainer->addElement(this->filePath);

        // Save button
        this->saveButton = new Aether::FilledButton(this->discNumber->x(), this->filePath->y(), this->discNumber->w(), 50, "Common.Save"_lang, 26, [this]() {
            this->saveChanges();
        });
        this->saveButton->setFillColour(this->app->theme()->accent());
        this->saveButton->setTextColour(Aether::Colour{0, 0, 0, 255});
        this->bottomContainer->addElement(this->saveButton);

        this->msgbox = nullptr;
    }

    void SongInfo::createInfoOverlay(const std::string & msg) {
        delete this->msgbox;
        this->msgbox = new Aether::MessageBox();
        this->msgbox->addTopButton("Common.OK"_lang, [this]() {
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

    void SongInfo::saveChanges() {
        // Don't permit blank name
        if (this->metadata.title.empty() || this->metadata.artist.empty() || this->metadata.album.empty()) {
            this->createInfoOverlay("Song.Information.Error"_lang);
            return;
        }

        // Commit changes to db (acquires lock and then writes)
        this->app->lockDatabase();
        bool ok = this->app->database()->updateSong(this->metadata);
        this->app->unlockDatabase();

        // Show message box indicating result
        this->createInfoOverlay((ok ? "Common.ChangesSaved"_lang : "Common.Error.DatabaseLocked"_lang));
    }

    void SongInfo::updateColours() {
        this->saveButton->setFillColour(this->app->theme()->accent());
    }

    SongInfo::~SongInfo() {
        if (this->msgbox) {
            this->msgbox->close();
        }
        delete this->msgbox;
    }
};