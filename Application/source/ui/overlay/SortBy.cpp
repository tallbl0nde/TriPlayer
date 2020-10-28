#include "ui/element/MenuButton.hpp"
#include "ui/overlay/SortBy.hpp"

// Padding top/bottom
constexpr int padding = 30;
// Overlay/list width
constexpr int maxHeight = 400;
constexpr int width = 400;

namespace CustomOvl {
    SortBy::SortBy(const std::string & title, const std::vector<Entry> & sort, std::function<void(Database::SortBy)> func) {
        this->callback = func;

        // Create and populate list
        this->list = new Aether::List(0, 0, width - padding, this->h(), Aether::Padding::FitScrollbar);
        size_t h = 0;
        for (size_t i = 0; i < sort.size(); i++) {
            CustomElm::MenuButton * b = new CustomElm::MenuButton();
            h = b->h();
            switch (sort[i].type) {
                case Database::SortBy::TitleAsc:
                    b->setIcon(new Aether::Image(0, 0, "romfs:/icons/musicnote.png"));
                    break;

                case Database::SortBy::TitleDsc:
                    b->setIcon(new Aether::Image(0, 0, "romfs:/icons/musicnote.png"));
                    break;

                case Database::SortBy::ArtistAsc:
                    b->setIcon(new Aether::Image(0, 0, "romfs:/icons/musicnote.png"));
                    break;

                case Database::SortBy::ArtistDsc:
                    b->setIcon(new Aether::Image(0, 0, "romfs:/icons/musicnote.png"));
                    break;

                case Database::SortBy::AlbumAsc:
                    b->setIcon(new Aether::Image(0, 0, "romfs:/icons/musicnote.png"));
                    break;

                case Database::SortBy::AlbumDsc:
                    b->setIcon(new Aether::Image(0, 0, "romfs:/icons/musicnote.png"));
                    break;

                case Database::SortBy::AlbumsAsc:
                    b->setIcon(new Aether::Image(0, 0, "romfs:/icons/musicnote.png"));
                    break;

                case Database::SortBy::AlbumsDsc:
                    b->setIcon(new Aether::Image(0, 0, "romfs:/icons/musicnote.png"));
                    break;

                case Database::SortBy::LengthAsc:
                    b->setIcon(new Aether::Image(0, 0, "romfs:/icons/musicnote.png"));
                    break;

                case Database::SortBy::LengthDsc:
                    b->setIcon(new Aether::Image(0, 0, "romfs:/icons/musicnote.png"));
                    break;

                case Database::SortBy::SongsAsc:
                    b->setIcon(new Aether::Image(0, 0, "romfs:/icons/musicnote.png"));
                    break;

                case Database::SortBy::SongsDsc:
                    b->setIcon(new Aether::Image(0, 0, "romfs:/icons/musicnote.png"));
                    break;
            }
            b->setText(sort[i].text);
            Entry entry = sort[i];
            b->setCallback([this, entry]() {
                this->close();
                this->callback(entry.type);
            });

            this->btns.push_back(b);
            this->list->addElement(b);
            if (i == 0) {
                b->setY(5);
            }
        }
        this->list->setH(h * (this->btns.size() + 1.4));
        this->list->setH(this->list->h() > maxHeight ? maxHeight : this->list->h());

        // Now create title
        this->heading = new Aether::Text(0, 0, title, 30);

        // Create background rectangle
        int total = this->list->h() + this->heading->h() + padding*2;
        this->bg = new Aether::Rectangle(this->x() + (this->w() - width)/2, this->y() + (this->h() - total)/2, width, total, 25);
        this->setTopLeft(bg->x(), bg->y());
        this->setBottomRight(this->x() + (this->w() + width)/2, this->y() + (this->h() + total)/2);
        this->addElement(this->bg);

        // Position everything
        this->addElement(this->heading);
        this->addElement(this->list);
        this->heading->setXY(this->bg->x() + padding, this->bg->y() + padding);
        this->list->setXY(this->bg->x() + padding/2, this->heading->y() + this->heading->h() + padding);
    }

    void SortBy::setBackgroundColour(const Aether::Colour & c) {
        this->bg->setColour(c);
    }

    void SortBy::setIconColour(const Aether::Colour & c) {
        this->list->setScrollBarColour(c);
        for (CustomElm::MenuButton * b : this->btns) {
            b->setIconColour(c);
        }
    }

    void SortBy::setTextColour(const Aether::Colour & c) {
        this->heading->setColour(c);
        for (CustomElm::MenuButton * b : this->btns) {
            b->setTextColour(c);
        }
    }
}