#include "element/Button.hpp"
#include "Utils.hpp"

namespace Element {
    Button::Button(short w, short h, const std::vector<uint8_t> & buf) : tsl::elm::Element(), colour(255, 255, 255, 255) {
        this->callback = nullptr;
        this->fontSize = 0;
        this->text = "";

        // Render image from buffer
        this->image = Utils::convertPNGToBitmap(buf, 0, 0);
        this->showAlt = false;

        // Set boundaries
        this->setBoundaries(this->getX(), this->getY(), this->image.width + w*2, this->image.height + h*2);
    }

    Button::Button(short w, short h, const std::string & str, unsigned int size) : tsl::elm::Element(), colour(255, 255, 255, 255) {
        this->fontSize = size;
        this->text = str;
        this->set = false;
        this->showAlt = false;

        // Set boundaries using empty text, which will be expanded later
        this->setBoundaries(this->getX(), this->getY(), w*2, h*2);
    }

    void Button::addAltImage(const std::vector<uint8_t> & buf) {
        this->altImage = Utils::convertPNGToBitmap(buf, 0, 0);
    }

    void Button::showAltImage(const bool b) {
        if (!this->altImage.pixels.empty()) {
            this->showAlt = b;
        }
    }

    void Button::setColour(tsl::Color col) {
        this->colour = col;
    }

    void Button::runCallback() {
        if (this->callback != nullptr) {
            this->callback();
        }
    }

    void Button::setCallback(std::function<void()> func) {
        this->callback = func;
    }

    tsl::elm::Element * Button::requestFocus(tsl::elm::Element * old, tsl::FocusDirection dir) {
        return this;
    }

    void Button::draw(tsl::gfx::Renderer * renderer) {
        // Draw string if set
        if (!this->text.empty()) {
            // Get dimensions of text and set size
            std::pair<u32, u32> dimensions = renderer->drawString(this->text.c_str(), false, 0, 0, this->fontSize, 0x0);
            if (!this->set) {
                this->setBoundaries(this->getX(), this->getY(), this->getWidth() + dimensions.first, this->getHeight() + dimensions.second);
                this->set = true;
            }

            // Draw text properly
            renderer->drawString(this->text.c_str(), false, this->getX() + (this->getWidth() - dimensions.first)/2, this->getY() + (this->getHeight() + dimensions.second - 6)/2, this->fontSize, tsl::style::color::ColorText);
            return;
        }

        // Set reference to determine what to draw
        Bitmap * img = &this->image;
        if (this->showAlt) {
            img = &this->altImage;
        }

        // Render if no error occurred
        if (!img->pixels.empty()) {
            int xOffset = this->getX() + (this->getWidth() - img->width)/2;
            int yOffset = this->getY() + (this->getHeight() - img->height)/2;

            // // Manually render so we can set the colour
            size_t idx = 0;
            for (size_t y = 0; y < img->height; y++) {
                for (size_t x = 0; x < img->width; x++) {
                    tsl::Color tmp = this->colour;
                    tmp.a = static_cast<u16>(img->pixels[idx + img->channels - 1] >> 4);
                    renderer->setPixelBlendSrc(xOffset + x, yOffset + y, renderer->a(tmp));
                    idx += img->channels;
                }
            }
        }
    }

    void Button::layout(u16 parentX, u16 parentY, u16 parentW, u16 parentH) {
        // Do nothing
    }

    bool Button::onClick(u64 keys) {
        if (keys & KEY_A) {
            this->runCallback();
            return true;
        }
        return false;
    }
};