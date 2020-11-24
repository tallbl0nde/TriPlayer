#include "Application.hpp"
#include "lang/Lang.hpp"
#include "meta/M3U.hpp"
#include "Paths.hpp"
#include "ui/element/listitem/Playlist.hpp"
#include "ui/frame/Playlists.hpp"
#include "ui/overlay/FileBrowser.hpp"
#include "ui/overlay/ItemMenu.hpp"
#include "ui/overlay/NewPlaylist.hpp"
#include "ui/overlay/SortBy.hpp"
#include "utils/FS.hpp"
#include "utils/Image.hpp"
#include "utils/Utils.hpp"

// New Playlist button dimensions
#define BUTTON_F 26
#define BUTTON_W 250
#define BUTTON_H 50

// Default paths for file browser
#define FILE_BROWSER_MUSIC "/music"
#define FILE_BROWSER_ROOT "/"
// Accepted file extensions
static const std::vector<std::string> FILE_EXTENSIONS_IMG = {".jpg", ".jpeg", ".jfif", ".png", ".JPG", ".JPEG", ".JFIF", ".PNG"};
static const std::vector<std::string> FILE_EXTENSIONS_M3U = {".m3u", ".m3u8"};

namespace Frame {
    Playlists::Playlists(Main::Application * a) : Frame(a) {
        // Remove headings (I should redo Frame to avoid this)
        this->topContainer->removeElement(this->titleH);
        this->topContainer->removeElement(this->artistH);
        this->topContainer->removeElement(this->albumH);
        this->topContainer->removeElement(this->lengthH);

        // Make the list larger as there's no heading
        this->list->setY(this->list->y() - 20);
        this->list->setH(this->list->h() + 20);

        // Now prepare this frame
        this->heading->setString("Playlist.Playlists"_lang);
        this->emptyMsg = nullptr;
        this->newButton = new Aether::FilledButton(this->x() + this->w() - BUTTON_W - 70, this->sort->y(), BUTTON_W, BUTTON_H, "Playlist.NewPlaylist"_lang, BUTTON_F, [this]() {
            this->createNewPlaylistMenu();
        });
        this->newButton->setFillColour(this->app->theme()->accent());
        this->newButton->setTextColour(Aether::Colour{0, 0, 0, 255});
        this->topContainer->addElement(this->newButton);

        Aether::BorderButton * btn = new Aether::BorderButton(this->newButton->x() - 70, this->newButton->y(), 50, 50, 2, "", BUTTON_F, [this]() {
            this->createFileBrowser(FILE_BROWSER_MUSIC, FILE_EXTENSIONS_M3U, "Playlist.SelectM3U"_lang);
        });
        btn->setBorderColour(this->app->theme()->FG());
        btn->setTextColour(this->app->theme()->FG());
        Aether::Image * img = new Aether::Image(btn->x() + btn->w()/2, btn->y() + btn->h()/2, "romfs:/icons/import.png");
        img->setXY(img->x() - img->w()/2, img->y() - img->h()/2);
        img->setColour(this->app->theme()->FG());
        btn->addElement(img);
        this->topContainer->addElement(btn);

        this->setHasSelectable(true);
        this->refreshList(Database::SortBy::TitleAsc);

        // Move sort button and prepare menu
        this->sort->setX(btn->x() - 20 - this->sort->w());
        this->sort->setCallback([this]() {
            this->app->addOverlay(this->sortMenu);
        });
        std::vector<CustomOvl::SortBy::Entry> sort = {{Database::SortBy::TitleAsc, "Playlist.Sort.TitleAltAsc"_lang},
                                                      {Database::SortBy::TitleDsc, "Playlist.Sort.TitleAltDsc"_lang},
                                                      {Database::SortBy::SongsAsc, "Playlist.Sort.SongsAsc"_lang},
                                                      {Database::SortBy::SongsDsc, "Playlist.Sort.SongsDsc"_lang}};
        this->sortMenu = new CustomOvl::SortBy("Playlist.Sort.HeadingAlt"_lang, sort, [this](Database::SortBy s) {
            this->refreshList(s);
        });
        this->sortMenu->setBackgroundColour(this->app->theme()->popupBG());
        this->sortMenu->setIconColour(this->app->theme()->muted());
        this->sortMenu->setLineColour(this->app->theme()->muted2());
        this->sortMenu->setTextColour(this->app->theme()->FG());

        this->checkFB = false;
        this->browser = nullptr;
        this->menu = nullptr;
        this->msgbox = nullptr;
        this->newMenu = nullptr;
        this->pushedIdx = -1;
    }

    CustomElm::ListItem::Playlist * Playlists::getListItem(const Metadata::Playlist & m) {
        CustomElm::ListItem::Playlist * l = new CustomElm::ListItem::Playlist(m.imagePath.empty() ? Path::App::DefaultPlaylistFile : m.imagePath);

        // Set styling parameters
        l->setNameString(m.name);
        l->setSongsString((m.songCount == 1 ? "Common.Song"_lang : Utils::substituteTokens("Common.Songs"_lang, std::to_string(m.songCount))));
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

    void Playlists::refreshList(Database::SortBy sort) {
        // Delete old data and fetch/create new items
        this->removeElement(this->emptyMsg);
        this->emptyMsg = nullptr;
        this->list->removeAllElements();
        this->items.clear();
        std::vector<Metadata::Playlist> m = this->app->database()->getAllPlaylistMetadata(sort);
        this->sortType = sort;

        // Show list if there are playlists
        if (m.size() > 0) {
            this->list->setHidden(false);
            this->subHeading->setString(m.size() == 1 ? "Playlist.CountOne"_lang : Utils::substituteTokens("Playlist.CountMany"_lang, std::to_string(m.size())));

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

        // Show message if no playlists
        } else {
            this->subHeading->setHidden(true);
            this->setFocussed(this->topContainer);
            this->emptyMsg = new Aether::Text(0, this->list->y() + this->list->h()*0.4, "Playlist.NotFound"_lang, 24);
            this->emptyMsg->setColour(this->app->theme()->FG());
            this->emptyMsg->setX(this->x() + (this->w() - this->emptyMsg->w())/2);
            this->addElement(this->emptyMsg);
        }
    }

    void Playlists::createDeletePlaylistMenu(const size_t pos) {
        delete this->msgbox;
        this->msgbox = new Aether::MessageBox();
        this->msgbox->addLeftButton("Common.Cancel"_lang, [this]() {
            // Do nothing; just close this overlay
            this->msgbox->close();
        });
        this->msgbox->addRightButton("Common.Delete"_lang, [this, pos]() {
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
                    this->refreshList(this->sortType);
                } else {
                    this->subHeading->setString(this->items.size() == 1 ? "Playlist.CountOne"_lang : Utils::substituteTokens("Playlist.CountMany"_lang, std::to_string(this->items.size())));
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
        tips->setString(Utils::substituteTokens("Playlist.DeletePromptName"_lang, this->items[pos].meta.name));
        tips->setColour(this->app->theme()->FG());
        body->addElement(tips);
        body->setH(tips->h() + 80);
        this->msgbox->setBody(body);
        this->msgbox->setBodySize(body->w(), body->h());
        this->app->addOverlay(this->msgbox);
    }

    void Playlists::createFileBrowser(const std::string & path, const std::vector<std::string> & exts, const std::string & heading) {
        delete this->browser;
        this->browser = new CustomOvl::FileBrowser(700, 600);
        this->browser->setAccentColour(this->app->theme()->accent());
        this->browser->setMutedLineColour(this->app->theme()->muted2());
        this->browser->setMutedTextColour(this->app->theme()->muted());
        this->browser->setRectangleColour(this->app->theme()->popupBG());
        this->browser->setTextColour(this->app->theme()->FG());
        this->browser->setCancelText("Common.Cancel"_lang);
        this->browser->setHeadingText(heading);
        this->browser->setExtensions(exts);

        // Set path and show
        this->checkFB = true;
        this->browser->setPath(path);
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
        std::string str = (this->items[pos].meta.songCount == 1 ? "Common.Song" : Utils::substituteTokens("Common.Songs"_lang, std::to_string(this->items[pos].meta.songCount)));
        this->menu->setSubText(str);

        // Play
        CustomElm::MenuButton * b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/playsmall.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Common.Play"_lang);
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, pos]() {
            std::vector<Metadata::PlaylistSong> v = this->app->database()->getSongMetadataForPlaylist(this->items[pos].meta.ID, Database::SortBy::TitleAsc);
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
        b->setText("Common.AddToQueue"_lang);
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, pos]() {
            std::vector<Metadata::PlaylistSong> v = this->app->database()->getSongMetadataForPlaylist(this->items[pos].meta.ID, Database::SortBy::TitleAsc);
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
        b->setText("Playlist.AddToOtherPlaylist"_lang);
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, pos]() {
            this->showAddToPlaylist([this, pos](PlaylistID i) {
                if (i >= 0) {
                    // Get list of songs and add one-by-one to other playlist
                    std::vector<Metadata::PlaylistSong> v = this->app->database()->getSongMetadataForPlaylist(this->items[pos].meta.ID, Database::SortBy::TitleAsc);
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
                        this->refreshList(this->sortType);
                    }

                    this->menu->close();
                }
            });
        });
        this->menu->addButton(b);
        this->menu->addSeparator(this->app->theme()->muted2());

        // Export playlist
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/export.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Playlist.ExportPlaylist"_lang);
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, pos]() {
            this->exportPlaylist(this->items[pos].meta);
        });
        this->menu->addButton(b);

        // Delete playlist
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/bin.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Playlist.DeletePlaylist"_lang);
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
        b->setText("Common.ViewInformation"_lang);
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
        this->newMenu->setHeading("Playlist.CreatePlaylist"_lang);
        this->newMenu->setNameString("Playlist.Information.Name"_lang);
        this->newMenu->setOKString("Playlist.Create"_lang);
        this->newMenu->setCancelString("Common.Cancel"_lang);
        this->newMenu->setAccentColour(this->app->theme()->accent());
        this->newMenu->setBackgroundColour(this->app->theme()->popupBG());
        this->newMenu->setHeadingColour(this->app->theme()->FG());
        this->newMenu->setTextBoxColour(this->app->theme()->muted2());
        this->newMenu->setTextColour(this->app->theme()->FG());
        this->newMenu->setImageCallback([this]() {
            this->createFileBrowser(FILE_BROWSER_ROOT, FILE_EXTENSIONS_IMG, "Common.SelectImage"_lang);
        });
        this->newMenu->setNameCallback([this](std::string name) {
            this->newData.name = name;
        });
        this->newMenu->setOKCallback([this]() {
            // Don't permit blank name
            if (this->newData.name.empty()) {
                this->createInfoOverlay("Common.Error.BlankName"_lang);
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

    void Playlists::exportPlaylist(const Metadata::Playlist & playlist) {
        // Fetch all metadata for the playlist's songs
        std::vector<Metadata::PlaylistSong> songs = this->app->database()->getSongMetadataForPlaylist(playlist.ID, Database::SortBy::TitleAsc);

        // Get a list of all file paths and make relative to root music folder
        Metadata::M3U::Playlist m3u;
        m3u.name = playlist.name;
        for (const Metadata::PlaylistSong & meta : songs) {
            // I'm assuming it's always "/music/" here, so we remove the first 7 characters
            if (meta.song.path.length() > 7) {
                m3u.paths.push_back(meta.song.path.substr(7, meta.song.path.length() - 7));
            }
        }

        // Write to file (we need to strip all non-supported FAT32 characters)
        std::string safeName = Utils::removeUnicode(playlist.name);
        size_t c = 0;
        while (c < safeName.length()) {
            char ch = safeName[c];
            if (ch == '\\' || ch == '/' || ch == ':' || ch == '*' || ch == '"' || ch == '<' || ch == '>' || ch == '|') {
                safeName.erase(c, 1);
            } else {
                c++;
            }
        }
        safeName = "/music/" + safeName + ".m3u8";
        bool ok = Metadata::M3U::writeFile(safeName, m3u);

        // Display error/success message
        if (ok) {
            this->createInfoOverlay(Utils::substituteTokens("Playlist.ExportSuccess"_lang, safeName));
        } else {
            this->createInfoOverlay("Playlist.ExportError"_lang);
        }
    }

    void Playlists::importPlaylist(const std::string & path) {
        // Read file, ensuring we have at least one song
        Metadata::M3U::Playlist playlist;
        bool ok = Metadata::M3U::parseFile(path, playlist);
        if (!ok || playlist.paths.empty()) {
            this->createInfoOverlay("Playlist.ImportError"_lang);
            return;
        }

        // Set default name if none found
        if (playlist.name.empty()) {
            playlist.name = Utils::Fs::getStem(path);
        }

        // Get the parent directory of the playlist file and append to ant relative paths
        // (we need absolute paths for the database)
        std::string parent = Utils::Fs::getParentDirectory(path) + "/";
        for (std::string & path : playlist.paths) {
            if (!path.empty() && path[0] != '/') {
                path = parent + path;
            }
        }

        // Now get an ID for each song path
        size_t failed = 0;
        std::vector<SongID> ids;
        for (std::string & path : playlist.paths) {
            // Check if file exists
            if (!Utils::Fs::fileExists(path)) {
                failed++;
                continue;
            }

            // Get ID for path
            SongID id = this->app->database()->getSongIDForPath(path);
            if (id < 0) {
                failed++;
                continue;
            }

            ids.push_back(id);
        }

        // Finally create playlist and add songs
        Metadata::Playlist meta;
        meta.name = playlist.name;
        this->app->lockDatabase();
        ok = this->app->database()->addPlaylist(meta);
        if (!ok) {
            this->createInfoOverlay("Common.Error.DatabaseLocked"_lang);
            this->app->unlockDatabase();
            return;
        }

        std::vector<Metadata::Playlist> plists = this->app->database()->getAllPlaylistMetadata(Database::SortBy::SongsAsc);
        for (const Metadata::Playlist & plist : plists) {
            if (meta.name == plist.name) {
                meta.ID = plist.ID;
                break;
            }
        }

        for (const SongID id : ids) {
            ok = this->app->database()->addSongToPlaylist(meta.ID, id);
            if (!ok) {
                break;
            }
        }
        this->app->unlockDatabase();

        // Inform user of result
        this->refreshList(this->sortType);
        if (failed != 0 || !ok) {
            this->createInfoOverlay(Utils::substituteTokens("Playlist.ImportSuccessSome"_lang, meta.name, std::to_string(failed)));
        } else {
            this->createInfoOverlay(Utils::substituteTokens("Playlist.ImportSuccess"_lang, meta.name, std::to_string(ids.size())));
        }
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
            this->refreshList(this->sortType);

        // Otherwise show a message
        } else {
            this->createInfoOverlay("Common.Error.DatabaseLocked"_lang);
        }
    }

    void Playlists::update(uint32_t dt) {
        Frame::update(dt);

        // Check the file browser result and create popup/image
        if (this->checkFB) {
            if (this->browser->shouldClose()) {
                std::string path = this->browser->chosenFile();
                if (!path.empty()) {
                    // Check if .m3u
                    bool isM3U = false;
                    std::string tmp = Utils::Fs::getExtension(path);
                    for (const std::string & ext : FILE_EXTENSIONS_M3U) {
                        if (tmp == ext) {
                            isM3U = true;
                            break;
                        }
                    }
                    if (isM3U) {
                        this->importPlaylist(path);

                    // Otherwise assume it's an image
                    } else {
                        // Attempt to create new image
                        Aether::Image * tmpImage = new Aether::Image(0, 0, path);

                        // Show error if image wasn't created
                        if (tmpImage->texW() == 0 || tmpImage->texH() == 0) {
                            this->createInfoOverlay("Common.Error.ReadImage"_lang);
                            delete tmpImage;

                        } else {
                            this->newData.imagePath = path;
                            this->newMenu->setImage(tmpImage);
                        }
                    }
                }
                this->checkFB = false;
            }
        }
    }

    void Playlists::onPop(Type t) {
        // Get new metadata (if last playlist was deleted we can simply refresh the list)
        std::vector<Metadata::Playlist> m = this->app->database()->getAllPlaylistMetadata(this->sortType);
        if (!this->items.empty() && m.empty()) {
            this->refreshList(this->sortType);
            return;
        }

        // If there's a count difference then we need to remove the pushed playlist (as it was deleted)
        if (m.size() != this->items.size()) {
            this->subHeading->setString(m.size() == 1 ? "Playlist.CountOne"_lang : Utils::substituteTokens("Playlist.CountMany"_lang, std::to_string(m.size())));
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
        delete this->sortMenu;
    }
};