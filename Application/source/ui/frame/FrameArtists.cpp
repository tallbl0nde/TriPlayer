#include "Application.hpp"
#include "ui/element/ListArtist.hpp"
#include "ui/element/ScrollableGrid.hpp"
#include "ui/frame/FrameArtists.hpp"
#include "utils/Utils.hpp"

// Number of ListArtists per row
#define COLUMNS 3

namespace Frame {
    Artists::Artists(Main::Application * a) : Frame(a) {
        // Remove list + headings (I should redo Frame to avoid this)
        this->removeElement(this->list);
        this->removeElement(this->titleH);
        this->removeElement(this->artistH);
        this->removeElement(this->albumH);
        this->removeElement(this->lengthH);

        // Now prepare this frame
        this->heading->setString("Artists");
        CustomElm::ScrollableGrid * grid = new CustomElm::ScrollableGrid(this->x(), this->y() + 150, this->w() - 10, this->h() - 150, 250, 3);
        grid->setShowScrollBar(true);
        grid->setScrollBarColour(this->app->theme()->muted2());

        // Create items for artists (note: this completely breaks how lists are supposed to be used)
        std::vector<Metadata::Artist> m = this->app->database()->getAllArtistMetadata();
        if (m.size() > 0) {
            for (size_t i = 0; i < m.size(); i++) {
                CustomElm::ListArtist * l = new CustomElm::ListArtist("romfs:/misc/noartist.png");
                l->setNameString(m[i].name);
                std::string str = std::to_string(m[i].albumCount) + (m[i].albumCount == 1 ? " album" : " albums");
                str += " | " + std::to_string(m[i].songCount) + (m[i].songCount == 1 ? " song" : " songs");
                l->setCountsString(str);
                l->setDotsColour(this->app->theme()->muted());
                l->setTextColour(this->app->theme()->FG());
                l->setMutedTextColour(this->app->theme()->muted());
                l->setCallback([this, i](){
                    // Change to artist's page
                });
                ArtistID id = m[i].ID;
                l->setMoreCallback([this, id]() {
                    // Show a menu?
                });
                grid->addElement(l);
            }

            this->subLength->setHidden(true);
            this->subTotal->setString(std::to_string(m.size()) + (m.size() == 1 ? " artist" : " artists" ));
            this->subTotal->setX(this->x() + 885 - this->subTotal->w());

            this->addElement(grid);
            this->setFocussed(grid);

        // Show message if no artists
        } else {
            grid->setHidden(true);
            this->subLength->setHidden(true);
            this->subTotal->setHidden(true);
            Aether::Text * emptyMsg = new Aether::Text(0, grid->y() + grid->h()*0.4, "No artists found!", 24);
            emptyMsg->setColour(this->app->theme()->FG());
            emptyMsg->setX(this->x() + (this->w() - emptyMsg->w())/2);
            this->addElement(emptyMsg);
        }
    }
};