#include "Application.hpp"
#include "ui/element/listitem/Playlist.hpp"
#include "ui/frame/Playlists.hpp"
#include "ui/overlay/FileBrowser.hpp"
#include "ui/overlay/ItemMenu.hpp"
#include "ui/overlay/NewPlaylist.hpp"
#include "utils/FS.hpp"
#include "utils/Utils.hpp"

// New Playlist button dimensions
#define BUTTON_F 26
#define BUTTON_W 250
#define BUTTON_H 50

// Location of default image
#define DEFAULT_IMAGE "romfs:/misc/noplaylist.png"
// Default path for file browser
#define FILE_BROWSER_ROOT "/"
// Accepted image extensions
static const std::vector<std::string> FILE_EXTENSIONS = {".jpg", ".jpeg", ".jfif", ".png", ".JPG", ".JPEG", ".JFIF", ".PNG"};
// Save location of images
#define SAVE_LOCATION "/switch/TriPlayer/images/playlist/"

namespace Frame {
    Playlists::Playlists(Main::Application * a) : Frame(a) {
        // Remove headings (I should redo Frame to avoid this)
        this->removeElement(this->titleH);
        this->removeElement(this->artistH);
        this->removeElement(this->albumH);
        this->removeElement(this->lengthH);

        // Make the list larger as there's no heading
        this->list->setY(this->list->y() - 20);
        this->list->setH(this->list->h() + 20);

        // Now prepare this frame
        this->heading->setString("Playlists");
        this->subLength->setHidden(true);
        this->subTotal->setHidden(true);
        this->emptyMsg = nullptr;
        this->newButton = new Aether::FilledButton(this->x() + this->w() - 320, this->heading->y() + (this->heading->h() - BUTTON_H)/2 + 5, BUTTON_W, BUTTON_H, "New Playlist", BUTTON_F, [this]() {
            this->createNewPlaylistMenu();
        });
        this->newButton->setFillColour(this->app->theme()->accent());
        this->newButton->setTextColour(Aether::Colour{0, 0, 0, 255});
        this->addElement(this->newButton);
        this->refreshList();

        this->checkFB = false;
        this->browser = nullptr;
        this->menu = nullptr;
        this->msgbox = nullptr;
        this->newMenu = nullptr;
    }

    void Playlists::refreshList() {
        this->list->removeAllElements();
        this->metadata = this->app->database()->getAllPlaylistMetadata();

        // Show list if there are playlists
        if (this->metadata.size() > 0) {
            this->list->setHidden(false);
            this->removeElement(this->emptyMsg);
            this->emptyMsg = nullptr;

            // Create list items for each playlist
            for (size_t i = 0; i < this->metadata.size(); i++) {
                std::string img = (this->metadata[i].imagePath.empty() ? DEFAULT_IMAGE : this->metadata[i].imagePath);
                CustomElm::ListItem::Playlist * l = new CustomElm::ListItem::Playlist(img);
                l->setNameString(this->metadata[i].name);
                std::string str = std::to_string(this->metadata[i].songCount) + (this->metadata[i].songCount == 1 ? " song" : " songs");
                l->setSongsString(str);
                l->setLineColour(this->app->theme()->muted2());
                l->setMoreColour(this->app->theme()->muted());
                l->setTextColour(this->app->theme()->FG());
                l->setMutedTextColour(this->app->theme()->muted());
                l->setCallback([this, i](){
                    // this->changeFrame(Type::Playlist, Action::Push, this->metadata[i].ID);
                });
                l->setMoreCallback([this, i]() {
                    this->createMenu(i);
                });
                this->list->addElement(l);

                if (i == 0) {
                    l->setY(this->list->y() + 10);
                }
            }
            this->setFocussed(this->list);

        // Show message if no playlists
        } else {
            this->list->setHidden(true);
            this->emptyMsg = new Aether::Text(0, this->list->y() + this->list->h()*0.4, "No playlists found, use the button above to create one!", 24);
            this->emptyMsg->setColour(this->app->theme()->FG());
            this->emptyMsg->setX(this->x() + (this->w() - this->emptyMsg->w())/2);
            this->addElement(this->emptyMsg);
            this->setFocussed(this->newButton);
        }
    }

    void Playlists::update(uint32_t dt) {
        Frame::update(dt);

        // Check the file browser result and create popup/image
        if (this->checkFB) {
            if (this->browser->shouldClose()) {
                std::string path = this->browser->chosenFile();
                if (!path.empty()) {
                    // Attempt to create new image
                    Aether::Image * tmpImage = new Aether::Image(0, 0, path);

                    // Show error if image wasn't created
                    if (tmpImage->texW() == 0 || tmpImage->texH() == 0) {
                        this->createInfoOverlay("An error occurred reading the selected image. This may be due to a corrupted image or incorrect file extension.");
                        delete tmpImage;

                    } else {
                        this->newData.imagePath = path;
                        this->newMenu->setImage(tmpImage);
                    }
                }
                this->checkFB = false;
            }
        }
    }

    void Playlists::createDeletePlaylistMenu(const size_t pos) {
        delete this->msgbox;
        this->msgbox = new Aether::MessageBox();
        this->msgbox->addLeftButton("Cancel", [this]() {
            // Do nothing; just close this overlay
            this->msgbox->close();
        });
        this->msgbox->addRightButton("Delete", [this, pos]() {
            // Delete and update list
            this->app->database()->close();
            this->app->sysmodule()->waitRequestDBLock();
            this->app->database()->openReadWrite();
            this->app->database()->removePlaylist(this->metadata[pos].ID);
            this->app->database()->close();
            this->app->sysmodule()->sendReleaseDBLock();
            this->app->database()->openReadOnly();
            this->refreshList();

            // Close both overlays
            this->msgbox->close();
            this->menu->close();
        });
        this->msgbox->setLineColour(this->app->theme()->muted2());
        this->msgbox->setRectangleColour(this->app->theme()->popupBG());
        this->msgbox->setTextColour(this->app->theme()->accent());
        Aether::Element * body = new Aether::Element(0, 0, 700);
        Aether::TextBlock * tips = new Aether::TextBlock(40, 40, "", 24, 620);
        tips->setString("Are you sure you want to delete the playlist '" + this->metadata[pos].name + "'? This action cannot be undone!");
        tips->setColour(this->app->theme()->FG());
        body->addElement(tips);
        body->setH(tips->h() + 80);
        this->msgbox->setBody(body);
        this->msgbox->setBodySize(body->w(), body->h());
        this->app->addOverlay(this->msgbox);
    }

    void Playlists::createFileBrowser() {
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

    void Playlists::createMenu(size_t pos) {
        delete this->menu;
        this->menu = new CustomOvl::ItemMenu();
        this->menu->setBackgroundColour(this->app->theme()->popupBG());
        this->menu->setMainTextColour(this->app->theme()->FG());
        this->menu->setSubTextColour(this->app->theme()->muted());
        this->menu->addSeparator(this->app->theme()->muted2());

        // Set playlist specific things
        this->menu->setImage(new Aether::Image(0, 0, this->metadata[pos].imagePath.empty() ? "romfs:/misc/noplaylist.png" : this->metadata[pos].imagePath));
        this->menu->setMainText(this->metadata[pos].name);
        std::string str = std::to_string(this->metadata[pos].songCount) + (this->metadata[pos].songCount == 1 ? " song" : " songs");
        this->menu->setSubText(str);

        // Play
        CustomElm::MenuButton * b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/playsmall.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Play");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, pos]() {
            std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForPlaylist(this->metadata[pos].ID);
            if (v.size() > 0) {
                std::vector<SongID> ids;
                for (size_t i = 0; i < v.size(); i++) {
                    ids.push_back(v[i].ID);
                }
                this->app->sysmodule()->sendSetPlayingFrom(this->metadata[pos].name);
                this->app->sysmodule()->sendSetQueue(ids);
                this->app->sysmodule()->sendSetSongIdx(0);
                this->app->sysmodule()->sendSetShuffle(this->app->sysmodule()->shuffleMode());
            }
            this->menu->close();
        });
        this->menu->addButton(b);
        this->menu->addSeparator(this->app->theme()->muted2());

        // Add to Queue
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoqueue.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Add to Queue");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, pos]() {
            std::vector<Metadata::Song> v = this->app->database()->getSongMetadataForPlaylist(this->metadata[pos].ID);
            for (size_t i = 0; i < v.size(); i++) {
                this->app->sysmodule()->sendAddToSubQueue(v[i].ID);
            }
            this->menu->close();
        });
        this->menu->addButton(b);

        // Add songs to another Playlist
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoplaylist.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Add to other Playlist");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, pos]() {
            // Do something
        });
        this->menu->addButton(b);
        this->menu->addSeparator(this->app->theme()->muted2());

        // Delete playlist
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/bin.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Delete Playlist");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, pos]() {
            this->createDeletePlaylistMenu(pos);
        });
        this->menu->addButton(b);
        this->menu->addSeparator(this->app->theme()->muted2());

        // View Information
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/info.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("View Information");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, pos]() {
            // this->changeFrame(Type::PlaylistInfo, Action::Push, this->metadata[pos].ID);
            // this->menu->close();
        });
        this->menu->addButton(b);

        // Finalize the menu
        this->menu->addButton(nullptr);
        this->app->addOverlay(this->menu);
    }

    void Playlists::createNewPlaylistMenu() {
        // Reset metadata
        this->newData.name = "";
        this->newData.description = "";
        this->newData.imagePath = "";

        // Create menu
        delete this->newMenu;
        this->newMenu = new CustomOvl::NewPlaylist();
        this->newMenu->setHeading("Create Playlist");
        this->newMenu->setNameString("Name");
        this->newMenu->setOKString("Create");
        this->newMenu->setCancelString("Cancel");
        this->newMenu->setAccentColour(this->app->theme()->accent());
        this->newMenu->setBackgroundColour(this->app->theme()->popupBG());
        this->newMenu->setHeadingColour(this->app->theme()->FG());
        this->newMenu->setTextBoxColour(this->app->theme()->muted2());
        this->newMenu->setTextColour(this->app->theme()->FG());
        this->newMenu->setImageCallback([this]() {
            this->createFileBrowser();
        });
        this->newMenu->setNameCallback([this](std::string name) {
            this->newData.name = name;
        });
        this->newMenu->setOKCallback([this]() {
            // Don't permit blank name
            if (this->newData.name.empty()) {
                this->createInfoOverlay("You can't have a blank name.");
            } else {
                this->savePlaylist();
                this->newMenu->close();
            }
        });

        this->app->addOverlay(this->newMenu);
    }

    void Playlists::createInfoOverlay(const std::string & msg) {
        delete this->msgbox;
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

    void Playlists::savePlaylist() {
        // Generate unique path (if you're really unlucky this could run indefinitely - but with 62^10 combinations we should be right :P)
        std::string src = this->newData.imagePath;
        if (!src.empty()) {
            std::string ext = Utils::Fs::getExtension(src);
            std::string rand;
            do {
                rand = Utils::randomString(10);
            } while (Utils::Fs::fileExists(SAVE_LOCATION + rand + ext));
            this->newData.imagePath = SAVE_LOCATION + rand + ext;
        }

        // Commit changes to db (acquires lock and then writes)
        this->app->database()->close();
        this->app->sysmodule()->waitRequestDBLock();
        this->app->database()->openReadWrite();
        bool ok = this->app->database()->addPlaylist(this->newData);
        this->app->database()->close();
        this->app->sysmodule()->sendReleaseDBLock();
        this->app->database()->openReadOnly();

        // If added ok actually manipulate image file(s)
        if (ok) {
            if (!src.empty()) {
                Utils::Fs::copyFile(src, this->newData.imagePath);
            }
            this->refreshList();

        // Otherwise show a message
        } else {
            this->createInfoOverlay("Unable to write to the database, see the logs for more information!");
        }
    }

    Playlists::~Playlists() {
        delete this->browser;
        delete this->menu;
        delete this->msgbox;
        delete this->newMenu;
    }
};