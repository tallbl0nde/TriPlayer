#include "gui/Error.hpp"

#define FONT_SIZE 22
#define ICON_SIZE 60
#define MESSAGE1 "An error occurred communicating\n"
#define MESSAGE2 "with the sysmodule. Check that\n"
#define MESSAGE3 "it is running and up-to-date!\n"

namespace Gui {
    tsl::elm::Element * Error::createUI() {
        // Root frame element
        tsl::elm::OverlayFrame * frame = new tsl::elm::OverlayFrame("TriPlayer", VER_STRING);

        // Error message (centered)
        tsl::elm::CustomDrawer * msg = new tsl::elm::CustomDrawer([](tsl::gfx::Renderer * renderer, u16 x, u16 y, u16 w, u16 h) {
            // Draw each line transparently first to get size, then center and draw
            std::pair<u32, u32> dimensions;
            u32 nextY = y + h*0.3;
            dimensions = renderer->drawString(":(\n", false, 0, 0, ICON_SIZE, 0x0);
            renderer->drawString(":(\n", false, x + (w - dimensions.first)/2, nextY, ICON_SIZE, 0xffff);
            nextY += dimensions.second * 1.15;

            dimensions = renderer->drawString(MESSAGE1, false, 0, 0, FONT_SIZE, 0x0);
            renderer->drawString(MESSAGE1, false, x + (w - dimensions.first)/2, nextY, FONT_SIZE, 0xffff);
            nextY += dimensions.second * 1.15;

            dimensions = renderer->drawString(MESSAGE2, false, 0, 0, FONT_SIZE, 0x0);
            renderer->drawString(MESSAGE2, false, x + (w - dimensions.first)/2, nextY, FONT_SIZE, 0xffff);
            nextY += dimensions.second * 1.15;

            dimensions = renderer->drawString(MESSAGE3, false, 0, 0, FONT_SIZE, 0x0);
            renderer->drawString(MESSAGE3, false, x + (w - dimensions.first)/2, nextY, FONT_SIZE, 0xffff);
        });

        frame->setContent(msg);
        return frame;
    }
};