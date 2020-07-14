#include "Application.hpp"
#include "ui/frame/Artist.hpp"

namespace Frame {
    Artist::Artist(Main::Application * app, ArtistID id) : Frame(app) {
        // First get metadata for the provided artist
        Metadata::Artist m = this->app->database()->getArtistMetadataForID(id);
        if (m.ID < 0) {
            // Helps show there was an error (should never appear)
            this->heading->setString("Artist");
            return;
        }

        // Otherwise populate with Artist's data
        Aether::Image * image = new Aether::Image(this->x() + 50, this->y() + 50, m.imagePath.empty() ? "romfs:/misc/noartist.png" : m.imagePath);
        image->setWH(100, 100);
        this->addElement(image);
        this->heading->setString(m.name);
        this->heading->setX(image->x() + image->w() + 20);
        std::string str = std::to_string(m.albumCount) + (m.albumCount == 1 ? " album" : " albums");
        str += " | " + std::to_string(m.songCount) + (m.songCount == 1 ? " song" : " songs");
        this->subTotal->setString(str);
    }
};