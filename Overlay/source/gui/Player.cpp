#include "Database.hpp"
#include "element/Player.hpp"
#include "gui/Player.hpp"
#include "ipc/TriPlayer.hpp"
#include "utils/FS.hpp"

namespace Gui {
    Player::Player(Database * db) {
        this->database = db;
        this->player = nullptr;
        this->currentSongID = -1;
        this->ticks = 0;
    }

    tsl::elm::Element * Player::createUI() {
        // Root frame element
        tsl::elm::OverlayFrame * frame = new tsl::elm::OverlayFrame("TriPlayer", " ");

        // Create single "player" element and set button callbacks
        this->player = new Element::Player();
        frame->setContent(this->player);

        // Force update to get initial metadata
        this->ticks = 100;
        this->update();

        return frame;
    }

    void Player::update() {
        // Only update 10 times per second
        if (this->ticks < 6) {
            this->ticks++;
            return;
        }
        this->ticks = 0;

        // Get currently playing song, and if changed update metadata
        int songID;
        if (!TriPlayer::getSongID(songID)) {
            return;
        }
        if (songID != this->currentSongID) {
            // Get metadata from database
            Metadata meta = this->database->getMetadataForID(songID);
            this->currentSongID = songID;
            if (meta.id < 0) {
                this->player->setTitle("Nothing playing!");
                this->player->setArtist("Play a song");
                this->player->setDuration(0);
                return;
            }

            // Set strings in element
            this->player->setTitle(meta.title);
            this->player->setArtist(meta.artist);
            this->player->setDuration(meta.duration);

            // Set new album art (an empty vector will cause default art to be shown)
            std::vector<uint8_t> buffer;
            if (!meta.imagePath.empty() && !Utils::Fs::readFile(meta.imagePath, buffer)) {
                buffer.clear();
            }
            this->player->setAlbumArt(buffer);
        }

        // Check playback status
        TriPlayer::Status status;
        if (!TriPlayer::getStatus(status)) {
            return;
        }
        this->player->setPlaying(status == TriPlayer::Status::Playing);

        // Check song position
        double pos;
        if (!TriPlayer::getPosition(pos)) {
            return;
        }
        this->player->setPosition(pos);

        // Check repeat
        TriPlayer::Repeat repeat;
        if (!TriPlayer::getRepeatMode(repeat)) {
            return;
        }
        this->player->setRepeat(repeat != TriPlayer::Repeat::Off, repeat == TriPlayer::Repeat::One);

        // Check shuffle
        TriPlayer::Shuffle shuffle;
        if (!TriPlayer::getShuffleMode(shuffle)) {
            return;
        }
        this->player->setShuffle(shuffle == TriPlayer::Shuffle::On);
    }
};