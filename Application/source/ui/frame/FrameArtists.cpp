#include "Application.hpp"
#include "ui/element/ListArtist.hpp"
#include "ui/frame/FrameArtists.hpp"
#include "ui/overlay/SongMenu.hpp"
#include "utils/Utils.hpp"

// Number of ListArtists per row
#define COLUMNS 3

namespace Frame {
    Artists::Artists(Main::Application * a) : Frame(a) {
        this->heading->setString("Artists");

        // Create items for artists
        std::vector<Metadata::Artist> m = this->app->database()->getAllArtistMetadata();
        if (m.size() > 0) {
            // Each container represents a row, which holds 3 ListArtists
            size_t rows = m.size()/COLUMNS + (m.size() % COLUMNS > 0 ? 1 : 0);
            for (size_t row = 0; row < rows; row++) {
                Aether::Container * c = new Aether::Container(0, 0, 1, 250);
                this->list->addElement(c);

                // Now create appropriate number of ListArtists and insert into row
                size_t num = (m.size() - (row * COLUMNS));
                num = (num > 3 ? 3 : num);
                for (size_t col = 0; col < num; col++) {
                    // Create and position item in container
                    CustomElm::ListArtist * l = new CustomElm::ListArtist(c->x() + (col/(float)COLUMNS)*(c->w()), c->y(), "romfs:/misc/noartist.png");

                    // Set strings and callbacks
                    size_t idx = (row * COLUMNS) + col;
                    l->setNameString(m[idx].name);
                    std::string str = std::to_string(m[idx].albumCount) + (m[idx].albumCount == 1 ? " album" : " albums");
                    str += " | " + std::to_string(m[idx].songCount) + (m[idx].songCount == 1 ? " song" : " songs");
                    l->setCountsString(str);
                    l->setDotsColour(this->app->theme()->muted());
                    l->setTextColour(this->app->theme()->FG());
                    l->setMutedTextColour(this->app->theme()->muted());
                    l->setCallback([this, idx](){
                        // Change to artist's page
                    });
                    ArtistID id = m[idx].ID;
                    l->setMoreCallback([this, id]() {
                        // Show a menu?
                    });
                    c->addElement(l);
                }

                if (row == 0) {
                    c->setY(this->list->y() + 10);
                }
            }

            this->subLength->setHidden(true);
            this->subTotal->setString(std::to_string(m.size()) + (m.size() == 1 ? " artist" : " artists" ));
            this->subTotal->setX(this->x() + 885 - this->subTotal->w());

            this->setFocussed(this->list);

        // Show message if no artists
        } else {
            this->list->setHidden(true);
            this->subLength->setHidden(true);
            this->subTotal->setHidden(true);
            Aether::Text * emptyMsg = new Aether::Text(0, this->list->y() + this->list->h()*0.4, "No artists found!", 24);
            emptyMsg->setColour(this->app->theme()->FG());
            emptyMsg->setX(this->x() + (this->w() - emptyMsg->w())/2);
            this->addElement(emptyMsg);
        }
    }
};