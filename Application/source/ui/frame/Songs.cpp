#include "Application.hpp"
#include "lang/Lang.hpp"
#include "Paths.hpp"
#include "ui/element/listitem/Song.hpp"
#include "ui/frame/Songs.hpp"
#include "ui/overlay/ItemMenu.hpp"
#include "ui/overlay/SortBy.hpp"
#include "utils/Utils.hpp"

namespace Frame {
    Songs::Songs(Main::Application * a) : Frame(a) {
        this->heading->setString("Song.Songs"_lang);
        this->createList(Database::SortBy::TitleAsc);

        // Set up sort overlay
        std::vector<CustomOvl::SortBy::Entry> sort = {{Database::SortBy::TitleAsc, "Song.Sort.TitleAsc"_lang},
                                                      {Database::SortBy::TitleDsc, "Song.Sort.TitleDsc"_lang},
                                                      {Database::SortBy::ArtistAsc, "Song.Sort.ArtistAsc"_lang},
                                                      {Database::SortBy::ArtistDsc, "Song.Sort.ArtistDsc"_lang},
                                                      {Database::SortBy::AlbumAsc, "Song.Sort.AlbumAsc"_lang},
                                                      {Database::SortBy::AlbumDsc, "Song.Sort.AlbumDsc"_lang},
                                                      {Database::SortBy::LengthAsc, "Song.Sort.LengthAsc"_lang},
                                                      {Database::SortBy::LengthDsc, "Song.Sort.LengthDsc"_lang}};
        this->sortMenu = new CustomOvl::SortBy("Song.Sort.Heading"_lang, sort, [this](Database::SortBy s) {
            this->createList(s);
        });
        this->sortMenu->setBackgroundColour(this->app->theme()->popupBG());
        this->sortMenu->setIconColour(this->app->theme()->muted());
        this->sortMenu->setLineColour(this->app->theme()->muted2());
        this->sortMenu->setTextColour(this->app->theme()->FG());
        this->sort->setCallback([this]() {
            this->app->addOverlay(this->sortMenu);
        });

        this->menu = nullptr;
    }

    void Songs::createList(Database::SortBy sort) {
        // Remove old items
        this->list->removeAllElements();
        this->songIDs.clear();

        // Create items for songs
        unsigned int totalSecs = 0;
        std::vector<Metadata::Song> m = this->app->database()->getAllSongMetadata(sort);
        if (m.size() > 0) {
            for (size_t i = 0; i < m.size(); i++) {
                this->songIDs.push_back(m[i].ID);
                totalSecs += m[i].duration;
                CustomElm::ListItem::Song * l = new CustomElm::ListItem::Song();
                l->setTitleString(m[i].title);
                l->setArtistString(m[i].artist);
                l->setAlbumString(m[i].album);
                l->setLengthString(Utils::secondsToHMS(m[i].duration));
                l->setLineColour(this->app->theme()->muted2());
                l->setMoreColour(this->app->theme()->muted());
                l->setTextColour(this->app->theme()->FG());
                l->setCallback([this, i](){
                    this->playNewQueue("Song.YourSongs"_lang, this->songIDs, i, false);
                });
                SongID id = m[i].ID;
                l->setMoreCallback([this, id]() {
                    this->createMenu(id);
                });
                this->list->addElement(l);

                if (i == 0) {
                    l->setY(this->list->y() + 10);
                }
            }

            // Set subheading
            std::string str;
            if (m.size() == 1) {
                str = Utils::substituteTokens("Song.DetailsOne"_lang, Utils::secondsToHoursMins(totalSecs));
            } else {
                str = Utils::substituteTokens("Song.DetailsMany"_lang, std::to_string(m.size()), Utils::secondsToHoursMins(totalSecs));
            }
            this->subHeading->setString(str);

        // Show message if no songs
        } else {
            this->list->setHidden(true);
            this->subHeading->setHidden(true);
            Aether::Text * emptyMsg = new Aether::Text(0, this->list->y() + this->list->h()*0.4, "Song.NotFound"_lang, 24);
            emptyMsg->setColour(this->app->theme()->FG());
            emptyMsg->setX(this->x() + (this->w() - emptyMsg->w())/2);
            this->addElement(emptyMsg);
        }
    }

    void Songs::createMenu(SongID id) {
        // Create menu
        delete this->menu;
        this->menu = new CustomOvl::ItemMenu();
        this->menu->setBackgroundColour(this->app->theme()->popupBG());
        this->menu->setMainTextColour(this->app->theme()->FG());
        this->menu->setSubTextColour(this->app->theme()->muted());
        this->menu->addSeparator(this->app->theme()->muted2());

        // Song metadata
        Metadata::Song m = this->app->database()->getSongMetadataForID(id);
        this->menu->setMainText(m.title);
        this->menu->setSubText(m.artist);
        AlbumID aID = this->app->database()->getAlbumIDForSong(m.ID);
        Metadata::Album md = this->app->database()->getAlbumMetadataForID(aID);
        this->menu->setImage(new Aether::Image(0, 0, md.imagePath.empty() ? Path::App::DefaultArtFile : md.imagePath));

        // Add to Queue
        CustomElm::MenuButton * b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoqueue.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Common.AddToQueue"_lang);
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, id]() {
            this->app->sysmodule()->sendAddToSubQueue(id);
            this->menu->close();
        });
        this->menu->addButton(b);

        // Add to Playlist
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/addtoplaylist.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Common.AddToPlaylist"_lang);
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, id]() {
            this->showAddToPlaylist([this, id](PlaylistID i) {
                if (i >= 0) {
                    this->app->database()->addSongToPlaylist(i, id);
                    this->menu->close();
                }
            });
        });
        this->menu->addButton(b);
        this->menu->addSeparator(this->app->theme()->muted2());

        // Go to Artist
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/user.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Common.GoToArtist"_lang);
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, id]() {
            ArtistID a = this->app->database()->getArtistIDForSong(id);
            if (a >= 0) {
                this->changeFrame(Type::Artist, Action::Push, a);
            }
            this->menu->close();
        });
        this->menu->addButton(b);

        // Go to Album
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/disc.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Common.GoToAlbum"_lang);
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, id]() {
            AlbumID a = this->app->database()->getAlbumIDForSong(id);
            if (a >= 0) {
                this->changeFrame(Type::Album, Action::Push, a);
            }
            this->menu->close();
        });
        this->menu->addButton(b);
        this->menu->addSeparator(this->app->theme()->muted2());

        // View Information
        b = new CustomElm::MenuButton();
        b->setIcon(new Aether::Image(0, 0, "romfs:/icons/info.png"));
        b->setIconColour(this->app->theme()->muted());
        b->setText("Common.ViewInformation"_lang);
        b->setTextColour(this->app->theme()->FG());
        b->setCallback([this, id]() {
            this->changeFrame(Type::SongInfo, Action::Push, id);
            this->menu->close();
        });
        this->menu->addButton(b);

        // Finalize the menu
        this->menu->addButton(nullptr);
        this->app->addOverlay(this->menu);
    }

    Songs::~Songs() {
        delete this->menu;
        delete this->sortMenu;
    }
};