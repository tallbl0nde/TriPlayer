#include "PlayerGui.hpp"

tsl::elm::Element * PlayerGui::createUI() {
    // Root frame element
    tsl::elm::OverlayFrame * frame = new tsl::elm::OverlayFrame("TriPlayer", VER_STRING);


    return frame;
}