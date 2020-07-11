#include "Application.hpp"
#include "ui/element/ListArtist.hpp"
#include "ui/frame/FrameArtists.hpp"
#include "ui/overlay/SongMenu.hpp"
#include "utils/Utils.hpp"

namespace Frame {
    Artists::Artists(Main::Application * a) : Frame(a) {
        this->heading->setString("Artists");

        // Create items for artists
        std::vector<Metadata::Artist> m = this->app->database()->getAllArtistMetadata();
        if (m.size() > 0) {
            for (size_t i = 0; i < m.size(); i++) {
                CustomElm::ListArtist * l = new CustomElm::ListArtist();
                l->setNameString(m[i].name);
                std::string str = std::to_string(m[i].albumCount) + (m[i].albumCount == 1 ? " album" : " albums");
                str += " | " + std::to_string(m[i].songCount) + (m[i].songCount == 1 ? " song" : " songs");
                l->setCountsString(str);
                l->setDotsColour(this->app->theme()->muted());
                l->setLineColour(this->app->theme()->muted2());
                l->setTextColour(this->app->theme()->FG());
                l->setMutedTextColour(this->app->theme()->muted());
                l->setCallback([this, i](){
                    // Change to artist's page
                });
                ArtistID id = m[i].ID;
                l->setMoreCallback([this, id]() {
                    // Show a menu?
                });
                this->list->addElement(l);

                if (i == 0) {
                    l->setY(this->list->y() + 10);
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