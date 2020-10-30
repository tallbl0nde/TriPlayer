#include "element/Player.hpp"
#include "gui/Player.hpp"

namespace Gui {
    tsl::elm::Element * Player::createUI() {
        // Root frame element
        tsl::elm::OverlayFrame * frame = new tsl::elm::OverlayFrame("TriPlayer", " ");

        // Create single "player" element
        Element::Player * player = new Element::Player();

        frame->setContent(player);
        return frame;
    }
};