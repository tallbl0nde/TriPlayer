#include "Application.hpp"
#include "lang/Lang.hpp"
#include <limits>
#include "meta/M3U.hpp"
#include "Paths.hpp"
#include "ui/element/listitem/Song.hpp"
#include "ui/frame/Playlist.hpp"
#include "ui/overlay/ItemMenu.hpp"
#include "ui/overlay/SortBy.hpp"
#include "utils/FS.hpp"
#include "utils/Utils.hpp"

// Play button dimensions
#define BUTTON_F 26
#define BUTTON_W 150
#define BUTTON_H 50

// Size of Playlist image
#define IMAGE_SIZE 160

namespace Frame {
    Playlist::Playlist(Main::Application * app, PlaylistID id) : Frame(app) {
        // Reposition elements
        this->albumH->setY(this->albumH->y() + 80);
        this->titleH->setY(this->albumH->y());
        this->artistH->setY(this->albumH->y());
        this->lengthH->setY(this->albumH->y());
        this->list->setY(this->list->y() + 80);
        this->list->setH(this->list->h() - 80);

        // First get metadata for the provided playlist
        this->metadata = this->app->database()->getPlaylistMetadataForID(id);
        if (this->metadata.ID < 0) {
            // Helps show there was an error (should never appear)
            this->heading->setString("Playlist.Playlist"_lang);
            this->msgbox = nullptr;
            this->playlistMenu = nullptr;
            this->songMenu = nullptr;
            this->sortMenu = nullptr;
            return;
        }

        // Populate with Playlist's data
        this->image = new Aether::Image(this->x() + 50, this->y() + 50, this->metadata.imagePath.empty() ? "romfs:/misc/noplaylist.png" : this->metadata.imagePath);
        this->image->setWH(IMAGE_SIZE, IMAGE_SIZE);
        this->addElement(this->image);
        this->heading->setString(this->metadata.name);
        this->heading->setX(image->x() + image->w() + 28);
        this->heading->setY(image->y() - 10);
        int maxW = (1280 - this->heading->x() - 30);
        if (this->heading->w() > maxW) {
            Aether::Text * tmp = new Aether::Text(this->heading->x() + maxW, this->heading->y(), "...", 60);
            tmp->setX(tmp->x() - tmp->w());
            this->addElement(tmp);
            this->heading->setW(maxW - tmp->w());
        }

        // Play and 'more' buttons
        this->playButton = new Aether::FilledButton(this->heading->x(), this->heading->y() + this->heading->h() + 45, BUTTON_W, BUTTON_H, "Common.Play"_lang, BUTTON_F, [this]() {
            this->playPlaylist(std::numeric_limits<size_t>::max());
        });
        this->playButton->setFillColour(this->app->theme()->accent());
        this->playButton->setTextColour(Aether::Colour{0, 0, 0, 255});
        this->sort->setY(this->playButton->y());

        Aether::BorderButton * moreButton = new Aether::BorderButton(this->playButton->x() + this->playButton->w() + 20, this->playButton->y(), BUTTON_H, BUTTON_H, 2, "", BUTTON_F, [this]() {
            this->createPlaylistMenu();
        });
        moreButton->setBorderColour(this->app->theme()->FG());
        moreButton->setTextColour(this->app->theme()->FG());
        Aether::Image * dots = new Aether::Image(moreButton->x() + moreButton->w()/2, moreButton->y() + moreButton->h()/2, "romfs:/icons/verticaldots.png");
        dots->setXY(dots->x() - dots->w()/2, dots->y() - dots->h()/2);
        dots->setColour(this->app->theme()->FG());
        moreButton->addElement(dots);

        this->topContainer->addElement(this->playButton);
        this->topContainer->addElement(moreButton);

        this->emptyMsg = nullptr;
        this->goBack = false;
        this->msgbox = nullptr;
        this->playlistMenu = nullptr;
        this->songMenu = nullptr;

        // Create sort menu
        this->sort->onPress([this]() {
            this->app->addOverlay(this->sortMenu);
        });
        std::vector<CustomOvl::SortBy::Entry> sort = {{Database::SortBy::TitleAsc, "Playlist.Sort.TitleAsc"_lang},
                                                      {Database::SortBy::TitleDsc, "Playlist.Sort.TitleDsc"_lang},
                                                      {Database::SortBy::ArtistAsc, "Playlist.Sort.ArtistAsc"_lang},
                                                      {Database::SortBy::ArtistDsc, "Playlist.Sort.ArtistDsc"_lang},
                                                      {Database::SortBy::AlbumAsc, "Playlist.Sort.AlbumAsc"_lang},
                                                      {Database::SortBy::AlbumDsc, "Playlist.Sort.AlbumDsc"_lang},
                                                      {Database::SortBy::LengthAsc, "Playlist.Sort.LengthAsc"_lang},
                                                      {Database::SortBy::LengthDsc, "Playlist.Sort.LengthDsc"_lang}};
        this->sortMenu = new CustomOvl::SortBy("Playlist.Sort.Heading"_lang, sort, [this](Database::SortBy s) {
            this->refreshList(s);
        });
        this->sortMenu->setBackgroundColour(this->app->theme()->popupBG());
        this->sortMenu->setIconColour(this->app->theme()->muted());
        this->sortMenu->setLineColour(this->app->theme()->muted2());
        this->sortMenu->setTextColour(this->app->theme()->FG());

        // Populate list
        this->refreshList(Database::SortBy::TitleAsc);
        this->setFocused(this->topContainer);
        this->topContainer->setFocused(this->playButton);
    }

    void Playlist::exportPlaylist() {
        // Get a list of all file paths and make relative to root music folder
        Metadata::M3U::Playlist m3u;
        m3u.name = this->metadata.name;
        for (const Metadata::PlaylistSong & meta : this->songs) {
            // I'm assuming it's always "/music/" here, so we remove the first 7 characters
            if (meta.song.path.length() > 7) {
                m3u.paths.push_back(meta.song.path.substr(7, meta.song.path.length() - 7));
            }
        }

        // Write to file (we need to strip all non-supported FAT32 characters)
        std::string safeName = Utils::removeUnicode(this->metadata.name);
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
        if (ok) {
            tips->setString(Utils::substituteTokens("Playlist.ExportSuccess"_lang, safeName));
        } else {
            tips->setString("Playlist.ExportError"_lang);
        }
        tips->setColour(this->app->theme()->FG());
        body->addElement(tips);
        body->setH(tips->h() + 80);
        this->msgbox->setBody(body);
        this->msgbox->setBodySize(body->w(), body->h());
        this->app->addOverlay(this->msgbox);
    }

    void Playlist::playPlaylist(size_t pos) {
        if (this->songs.size() > 0) {
            std::vector<SongID> ids;
            for (size_t i = 0; i < this->songs.size(); i++) {
                ids.push_back(this->songs[i].song.ID);
            }
            this->playNewQueue(this->metadata.name, ids, (pos == std::numeric_limits<size_t>::max() ? 0 : pos), (pos == std::numeric_limits<size_t>::max()));
        }
    }

    void Playlist::calculateStats() {
        // Show number of songs and duration
        unsigned int total = 0;
        for (size_t i = 0; i < this->songs.size(); i++) {
            total += this->songs[i].song.duration;
        }
        std::string str = Utils::secondsToHoursMins(total);
        if (this->songs.size() == 1) {
            str = Utils::substituteTokens("Playlist.DetailsOne"_lang, str);
        } else {
            str = Utils::substituteTokens("Playlist.DetailsMany"_lang, std::to_string(this->songs.size()), str);
        }
        this->subHeading->setString(str);
        this->subHeading->setXY(this->heading->x() + 2, this->heading->y() + this->heading->h());

        this->removeElement(this->emptyMsg);
        this->emptyMsg = nullptr;
        if (this->songs.size() == 0) {
            this->emptyMsg = new Aether::Text(0, this->list->y() + this->list->h()*0.4, "Playlist.NoSongs"_lang, 24);
            this->emptyMsg->setColour(this->app->theme()->FG());
            this->emptyMsg->setX(this->x() + (this->w() - emptyMsg->w())/2);
            this->addElement(this->emptyMsg);
            this->list->setHidden(true);
            this->setFocused(this->topContainer);
        }
    }

    void Playlist::refreshList(Database::SortBy sort) {
        this->sortType = sort;

        // Create list elements for each song
        this->elms.clear();
        this->list->removeAllElements();
        this->metadata = this->app->database()->getPlaylistMetadataForID(this->metadata.ID);
        this->songs = this->app->database()->getSongMetadataForPlaylist(this->metadata.ID, sort);
        if (this->songs.size() > 0) {
            for (size_t i = 0; i < this->songs.size(); i++) {
                CustomElm::ListItem::Song * l = new CustomElm::ListItem::Song();
                l->setTitleString(this->songs[i].song.title);
                l->setArtistString(this->songs[i].song.artist);
                l->setAlbumString(this->songs[i].song.album);
                l->setLengthString(Utils::secondsToHMS(this->songs[i].song.duration));
                l->setLineColour(this->app->theme()->muted2());
                l->setMoreColour(this->app->theme()->muted());
                l->setTextColour(this->app->theme()->FG());
                // Need to search for element as order is changed when a song is removed
                l->onPress([this, l](){
                    std::vector<CustomElm::ListItem::Song *>::iterator it = std::find(this->elms.begin(), this->elms.end(), l);
                    if (it != this->elms.end()) {
                        this->playPlaylist(std::distance(this->elms.begin(), it));
                    }
                });
                l->setMoreCallback([this, l]() {
                    std::vector<CustomElm::ListItem::Song *>::iterator it = std::find(this->elms.begin(), this->elms.end(), l);
                    if (it != this->elms.end()) {
                        this->createSongMenu(std::distance(this->elms.begin(), it));
                    }
                });
                this->list->addElement(l);
                this->elms.push_back(l);

                if (i == 0) {
                    l->setY(this->list->y() + 10);
                }
            }
        }

        this->calculateStats();
    }

    void Playlist::createDeleteMenu() {
        delete this->msgbox;
        this->msgbox = new Aether::MessageBox();
        this->msgbox->addLeftButton("Common.Cancel"_lang, [this]() {
            // Do nothing; just close this overlay
            this->msgbox->close();
        });
        this->msgbox->addRightButton("Common.Delete"_lang, [this]() {
            this->app->lockDatabase();
            bool ok = this->app->database()->removePlaylist(this->metadata.ID);
            this->app->unlockDatabase();

            // Delete image and frame if succeeded
            if (ok) {
                Utils::Fs::deleteFile(this->metadata.imagePath);
                this->goBack = true;
            }
        });
        this->msgbox->setLineColour(this->app->theme()->muted2());
        this->msgbox->setRectangleColour(this->app->theme()->popupBG());
        this->msgbox->setTextColour(this->app->theme()->accent());
        Aether::Element * body = new Aether::Element(0, 0, 700);
        Aether::TextBlock * tips = new Aether::TextBlock(40, 40, "", 24, 620);
        tips->setString("Playlist.DeletePrompt"_lang);
        tips->setColour(this->app->theme()->FG());
        body->addElement(tips);
        body->setH(tips->h() + 80);
        this->msgbox->setBody(body);
        this->msgbox->setBodySize(body->w(), body->h());
        this->app->addOverlay(this->msgbox);
    }

    void Playlist::createPlaylistMenu() {
        // Create if doesn't exist
        if (this->playlistMenu == nullptr) {
            this->playlistMenu = new CustomOvl::Menu();
            this->playlistMenu->setBackgroundColour(this->app->theme()->popupBG());

            // Add to Queue
            CustomElm::MenuButton * b = new CustomElm::MenuButton();
            b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoqueue.png"));
            b->setIconColour(this->app->theme()->muted());
            b->setText("Common.AddToQueue"_lang);
            b->setTextColour(this->app->theme()->FG());
            b->onPress([this]() {
                for (size_t i = 0; i < this->songs.size(); i++) {
                    this->app->sysmodule()->sendAddToSubQueue(this->songs[i].song.ID);
                }
                this->playlistMenu->close();
            });
            this->playlistMenu->addButton(b);

            // Add to other Playlist
            b = new CustomElm::MenuButton();
            b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoplaylist.png"));
            b->setIconColour(this->app->theme()->muted());
            b->setText("Playlist.AddToOtherPlaylist"_lang);
            b->setTextColour(this->app->theme()->FG());
            b->onPress([this]() {
                this->showAddToPlaylist([this](PlaylistID i) {
                    if (i >= 0) {
                        for (size_t j = 0; j < this->songs.size(); j++) {
                            this->app->database()->addSongToPlaylist(i, this->songs[j].song.ID);
                        }
                        this->playlistMenu->close();

                        // Refresh the list if it's this playlist
                        if (i == this->metadata.ID) {
                            this->refreshList(this->sortType);
                        }
                    }
                });
            });
            this->playlistMenu->addButton(b);
            this->playlistMenu->addSeparator(this->app->theme()->muted2());

            // Export playlist
            b = new CustomElm::MenuButton();
            b->setIcon(new Aether::Image(0, 0, "romfs:/icons/export.png"));
            b->setIconColour(this->app->theme()->muted());
            b->setText("Playlist.ExportPlaylist"_lang);
            b->setTextColour(this->app->theme()->FG());
            b->onPress([this]() {
                this->exportPlaylist();
            });
            this->playlistMenu->addButton(b);

            // Delete playlist
            b = new CustomElm::MenuButton();
            b->setIcon(new Aether::Image(0, 0, "romfs:/icons/bin.png"));
            b->setIconColour(this->app->theme()->muted());
            b->setText("Playlist.DeletePlaylist"_lang);
            b->setTextColour(this->app->theme()->FG());
            b->onPress([this]() {
                this->createDeleteMenu();
            });
            this->playlistMenu->addButton(b);
            this->playlistMenu->addSeparator(this->app->theme()->muted2());

            // View Information
            b = new CustomElm::MenuButton();
            b->setIcon(new Aether::Image(0, 0, "romfs:/icons/info.png"));
            b->setIconColour(this->app->theme()->muted());
            b->setText("Common.ViewInformation"_lang);
            b->setTextColour(this->app->theme()->FG());
            b->onPress([this]() {
                this->changeFrame(Type::PlaylistInfo, Action::Push, this->metadata.ID);
                this->playlistMenu->close();
            });
            this->playlistMenu->addButton(b);

            // Finalize the menu
            this->playlistMenu->addButton(nullptr);
        }

        this->playlistMenu->resetHighlight();
        this->app->addOverlay(this->playlistMenu);
    }

    void Playlist::createSongMenu(size_t pos) {
        // Create menu
        delete this->songMenu;
        this->songMenu = new CustomOvl::ItemMenu();
        this->songMenu->setBackgroundColour(this->app->theme()->popupBG());
        this->songMenu->setMainTextColour(this->app->theme()->FG());
        this->songMenu->setSubTextColour(this->app->theme()->muted());
        this->songMenu->addSeparator(this->app->theme()->muted2());

        // Song metadata
        this->songMenu->setMainText(this->songs[pos].song.title);
        this->songMenu->setSubText(this->songs[pos].song.artist);
        AlbumID id = this->app->database()->getAlbumIDForSong(this->songs[pos].song.ID);
        Metadata::Album md = this->app->database()->getAlbumMetadataForID(id);
        this->songMenu->setImage(new Aether::Image(0, 0, md.imagePath.empty() ? Path::App::DefaultArtFile : md.imagePath));

        // Add to Queue
        CustomElm::MenuButton * b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoqueue.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Common.AddToQueue"_lang);
        b->setTextColour(this->app->theme()->FG());
        b->onPress([this, pos]() {
            this->app->sysmodule()->sendAddToSubQueue(this->songs[pos].song.ID);
            this->songMenu->close();
        });
        this->songMenu->addButton(b);

        // Add to Playlist
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoplaylist.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Playlist.AddToOtherPlaylist"_lang);
        b->setTextColour(this->app->theme()->FG());
        b->onPress([this, pos]() {
            this->showAddToPlaylist([this, pos](PlaylistID i) {
                if (i >= 0) {
                    this->app->database()->addSongToPlaylist(i, this->songs[pos].song.ID);
                    this->songMenu->close();

                    // Refresh the list if it's this playlist
                    if (i == this->metadata.ID) {
                        this->refreshList(this->sortType);
                    }
                }
            });
        });
        this->songMenu->addButton(b);

        // Remove from Playlist
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/removefromplaylist.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Playlist.RemoveFromPlaylist"_lang);
        b->setTextColour(this->app->theme()->FG());
        b->onPress([this, pos]() {
            // Remove from database
            this->app->lockDatabase();
            bool ok = this->app->database()->removeSongFromPlaylist(this->songs[pos].ID);
            this->app->unlockDatabase();

            // Remove from lists
            if (ok) {
                this->list->removeElement(this->elms[pos]);
                this->elms.erase(this->elms.begin() + pos);
                this->songs.erase(this->songs.begin() + pos);
                this->calculateStats();
            }
            this->songMenu->close();
        });
        this->songMenu->addButton(b);
        this->songMenu->addSeparator(this->app->theme()->muted2());

        // Go to Artist
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/user.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Common.GoToArtist"_lang);
        b->setTextColour(this->app->theme()->FG());
        b->onPress([this, pos]() {
            ArtistID a = this->app->database()->getArtistIDForSong(this->songs[pos].song.ID);
            if (a >= 0) {
                this->changeFrame(Type::Artist, Action::Push, a);
            }
            this->songMenu->close();
        });
        this->songMenu->addButton(b);
        this->songMenu->addSeparator(this->app->theme()->muted2());

        // View Information
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/info.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Common.ViewInformation"_lang);
        b->setTextColour(this->app->theme()->FG());
        b->onPress([this, pos]() {
            this->changeFrame(Type::SongInfo, Action::Push, this->songs[pos].song.ID);
            this->songMenu->close();
        });
        this->songMenu->addButton(b);

        // Finalize the menu
        this->songMenu->addButton(nullptr);
        this->app->addOverlay(this->songMenu);
    }

    void Playlist::update(uint32_t dt) {
        // Update children first
        Frame::update(dt);

        // Change frame if flag set (only set if both the overlays are open)
        if (this->goBack) {
            this->msgbox->close();
            this->playlistMenu->close();
            this->changeFrame(Type::Playlists, Action::Back, 0);
        }
    }

    void Playlist::onPop(Type t) {
        // Only take action if frame was PlaylistInfo
        if (t == Type::PlaylistInfo) {
            // Get updated metadata and update elements if needed
            Metadata::Playlist m = this->app->database()->getPlaylistMetadataForID(this->metadata.ID);
            if (m.imagePath != this->metadata.imagePath) {
                this->removeElement(this->image);
                this->image = new Aether::Image(this->x() + 50, this->y() + 50, m.imagePath.empty() ? "romfs:/misc/noplaylist.png" : m.imagePath);
                this->image->setWH(IMAGE_SIZE, IMAGE_SIZE);
                this->addElement(this->image);
            }
            this->heading->setString(m.name);
            this->metadata = m;
        }
    }

    void Playlist::updateColours() {
        this->playButton->setFillColour(this->app->theme()->accent());
    }

    Playlist::~Playlist() {
        delete this->msgbox;
        delete this->playlistMenu;
        delete this->songMenu;
        delete this->sortMenu;
    }
};
