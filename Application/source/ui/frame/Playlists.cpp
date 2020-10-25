#include "Application.hpp"
#include "Paths.hpp"
#include "ui/element/listitem/Playlist.hpp"
#include "ui/frame/Playlists.hpp"
#include "ui/overlay/FileBrowser.hpp"
#include "ui/overlay/ItemMenu.hpp"
#include "ui/overlay/NewPlaylist.hpp"
#include "utils/FS.hpp"
#include "utils/Image.hpp"
#include "utils/Utils.hpp"

// New Playlist button dimensions
#define BUTTON_F 26
#define BUTTON_W 250
#define BUTTON_H 50

// Default path for file browser
#define FILE_BROWSER_ROOT "/"
// Accepted image extensions
static const std::vector<std::string> FILE_EXTENSIONS = {".jpg", ".jpeg", ".jfif", ".png", ".JPG", ".JPEG", ".JFIF", ".PNG"};

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
        this->pushedIdx = -1;
    }

    CustomElm::ListItem::Playlist * Playlists::getListItem(const Metadata::Playlist & m) {
        CustomElm::ListItem::Playlist * l = new CustomElm::ListItem::Playlist(m.imagePath.empty() ? Path::App::DefaultArtFile : m.imagePath);

        // Set styling parameters
        l->setNameString(m.name);
        l->setSongsString(std::to_string(m.songCount) + (m.songCount == 1 ? " song" : " songs"));
        l->setLineColour(this->app->theme()->muted2());
        l->setMoreColour(this->app->theme()->muted());
        l->setTextColour(this->app->theme()->FG());
        l->setMutedTextColour(this->app->theme()->muted());

        // Need to search for element in callbacks as order is changed when a playlist is removed
        l->setCallback([this, l](){
            std::vector<Item>::iterator it = std::find_if(this->items.begin(), this->items.end(), [this, l](const Item e) {
                return e.elm == l;
            });
            if (it != this->items.end()) {
                size_t idx = std::distance(this->items.begin(), it);
                this->pushedIdx = idx;
                this->changeFrame(Type::Playlist, Action::Push, this->items[idx].meta.ID);
            }
        });
        l->setMoreCallback([this, l]() {
            std::vector<Item>::iterator it = std::find_if(this->items.begin(), this->items.end(), [this, l](const Item e) {
                return e.elm == l;
            });
            if (it != this->items.end()) {
                size_t idx = std::distance(this->items.begin(), it);
                this->pushedIdx = idx;
                this->createMenu(idx);
            }
        });

        return l;
    }

    void Playlists::refreshList() {
        // Delete old data and fetch/create new items
        this->removeElement(this->emptyMsg);
        this->emptyMsg = nullptr;
        this->list->removeAllElements();
        this->items.clear();
        std::vector<Metadata::Playlist> m = this->app->database()->getAllPlaylistMetadata();

        // Show list if there are playlists
        if (m.size() > 0) {
            this->list->setHidden(false);

            // Create list items for each playlist
            for (size_t i = 0; i < m.size(); i++) {
                // Create and push element onto vector
                CustomElm::ListItem::Playlist * l = this->getListItem(m[i]);
                this->items.push_back(Item{m[i], l});

                // Position in list
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

    void Playlists::createDeletePlaylistMenu(const size_t pos) {
        delete this->msgbox;
        this->msgbox = new Aether::MessageBox();
        this->msgbox->addLeftButton("Cancel", [this]() {
            // Do nothing; just close this overlay
            this->msgbox->close();
        });
        this->msgbox->addRightButton("Delete", [this, pos]() {
            // Delete and update list
            this->app->lockDatabase();
            bool ok = this->app->database()->removePlaylist(this->items[pos].meta.ID);
            this->app->unlockDatabase();

            // Remove image and item from list if succeeded
            if (ok) {
                Utils::Fs::deleteFile(this->items[pos].meta.imagePath);
                this->list->removeElement(this->items[pos].elm);
                this->items.erase(this->items.begin() + pos);
                if (this->items.empty()) {
                    this->refreshList();
                }
            }

            // Close both overlays
            this->msgbox->close();
            this->menu->close();
        });
        this->msgbox->setLineColour(this->app->theme()->muted2());
        this->msgbox->setRectangleColour(this->app->theme()->popupBG());
        this->msgbox->setTextColour(this->app->theme()->accent());
        Aether::Element * body = new Aether::Element(0, 0, 700);
        Aether::TextBlock * tips = new Aether::TextBlock(40, 40, "", 24, 620);
        tips->setString("Are you sure you want to delete the playlist '" + this->items[pos].meta.name + "'? This action cannot be undone!");
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
        this->menu->setImage(new Aether::Image(0, 0, this->items[pos].meta.imagePath.empty() ? "romfs:/misc/noplaylist.png" : this->items[pos].meta.imagePath));
        this->menu->setMainText(this->items[pos].meta.name);
        std::string str = std::to_string(this->items[pos].meta.songCount) + (this->items[pos].meta.songCount == 1 ? " song" : " songs");
        this->menu->setSubText(str);

        // Play
        CustomElm::MenuButton * b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/playsmall.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Play");
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, pos]() {
            std::vector<Metadata::PlaylistSong> v = this->app->database()->getSongMetadataForPlaylist(this->items[pos].meta.ID);
            if (v.size() > 0) {
                std::vector<SongID> ids;
                for (size_t i = 0; i < v.size(); i++) {
                    ids.push_back(v[i].song.ID);
                }
                this->playNewQueue(this->items[pos].meta.name, ids, 0, true);
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
            std::vector<Metadata::PlaylistSong> v = this->app->database()->getSongMetadataForPlaylist(this->items[pos].meta.ID);
            for (size_t i = 0; i < v.size(); i++) {
                this->app->sysmodule()->sendAddToSubQueue(v[i].song.ID);
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
            this->showAddToPlaylist([this, pos](PlaylistID i) {
                if (i >= 0) {
                    // Get list of songs and add one-by-one to other playlist
                    std::vector<Metadata::PlaylistSong> v = this->app->database()->getSongMetadataForPlaylist(this->items[pos].meta.ID);
                    for (size_t j = 0; j < v.size(); j++) {
                        this->app->database()->addSongToPlaylist(i, v[j].song.ID);
                    }

                    // Recreate list item in order to update song count
                    std::vector<Item>::iterator it = std::find_if(this->items.begin(), this->items.end(), [this, i](const Item e) {
                        return e.meta.ID == i;
                    });
                    if (it != this->items.end()) {
                        size_t idx = std::distance(this->items.begin(), it);
                        this->list->removeElement(this->items[idx].elm);
                        this->items[idx].meta = this->app->database()->getPlaylistMetadataForID(i);
                        this->items[idx].elm = this->getListItem(this->items[idx].meta);
                        this->list->addElementAfter(this->items[idx].elm, (idx == 0 ? nullptr : this->items[idx-1].elm));

                    // Otherwise recreate list
                    } else {
                        this->refreshList();
                    }

                    this->menu->close();
                }
            });
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
            this->changeFrame(Type::PlaylistInfo, Action::Push, this->items[pos].meta.ID);
            this->menu->close();
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
            } while (Utils::Fs::fileExists(Path::App::PlaylistImageFolder + rand + ext));
            this->newData.imagePath = Path::App::PlaylistImageFolder + rand + ext;
        }

        // Commit changes to db (acquires lock and then writes)
        this->app->lockDatabase();
        bool ok = this->app->database()->addPlaylist(this->newData);
        this->app->unlockDatabase();

        // If added ok actually manipulate image file(s)
        if (ok) {
            if (!src.empty()) {
                std::vector<unsigned char> buffer;
                Utils::Fs::readFile(src, buffer);
                Utils::Image::resize(buffer, 400, 400);
                Utils::Fs::writeFile(this->newData.imagePath, buffer);
            }
            // Completely recreate list
            this->refreshList();

        // Otherwise show a message
        } else {
            this->createInfoOverlay("Unable to write to the database, see the logs for more information!");
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

    void Playlists::onPop(Type t) {
        // Get new metadata (if last playlist was deleted we can simply refresh the list)
        std::vector<Metadata::Playlist> m = this->app->database()->getAllPlaylistMetadata();
        if (!this->items.empty() && m.empty()) {
            this->refreshList();
            return;
        }

        // If there's a count difference then we need to remove the pushed playlist (as it was deleted)
        if (m.size() != this->items.size()) {
            this->list->removeElement(this->items[this->pushedIdx].elm);
            this->items.erase(this->items.begin() + this->pushedIdx);
            return;
        }

        // Recreate the playlist item if the metadata changed (and move to correct position)
        std::vector<Metadata::Playlist>::iterator it = std::find_if(m.begin(), m.end(), [this](const Metadata::Playlist m) {
            return m.ID == this->items[this->pushedIdx].meta.ID;
        });
        if (it != m.end()) {
            size_t idx = std::distance(m.begin(), it);

            // Check if we need to recreate the element due to metadata changes
            Metadata::Playlist o = this->items[this->pushedIdx].meta;
            if (o.imagePath != m[idx].imagePath || o.name != m[idx].name || o.songCount != m[idx].songCount) {
                // If so remove the element from the list
                bool hi = (this->list->focussed() == this->items[this->pushedIdx].elm);
                this->list->removeElement(this->items[this->pushedIdx].elm);
                this->items[this->pushedIdx].elm = this->getListItem(m[idx]);
                this->items[this->pushedIdx].meta = m[idx];

                // Now rotate items to match future positions;
                if (this->pushedIdx != idx) {
                    if (this->pushedIdx < idx) {
                        std::rotate(this->items.begin() + this->pushedIdx, this->items.begin() + this->pushedIdx + 1, this->items.begin() + idx + 1);
                    } else if (this->pushedIdx > idx) {
                        std::rotate(this->items.begin() + idx, this->items.begin() + this->pushedIdx, this->items.begin() + this->pushedIdx + 1);
                    }
                }

                // Finally (re)add to list
                this->list->addElementAfter(this->items[idx].elm, (idx == 0 ? nullptr : this->items[idx-1].elm));
                if (hi) {
                    this->list->setFocussed(this->items[idx].elm);
                }
            }
        }

        // Now that everything is in order, check if other element's song counts have changed
        for (size_t i = 0; i < this->items.size(); i++) {
            if (m[i].songCount != this->items[i].meta.songCount) {
                // Recreate if different count
                this->list->removeElement(this->items[i].elm);
                this->items[i].elm = this->getListItem(m[i]);
                this->items[i].meta = m[i];
                this->list->addElementAfter(this->items[i].elm, (i == 0 ? nullptr : this->items[i-1].elm));
            }
        }

        this->pushedIdx = -1;
    }

    void Playlists::updateColours() {
        this->newButton->setFillColour(this->app->theme()->accent());
    }

    Playlists::~Playlists() {
        delete this->browser;
        delete this->menu;
        delete this->msgbox;
        delete this->newMenu;
    }
};