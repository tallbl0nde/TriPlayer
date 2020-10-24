#include "ui/overlay/ComboPicker.hpp"

// Overlay panel height
constexpr int height = 450;
constexpr int padding = 140;
constexpr unsigned int titleFontSize = 26;
constexpr unsigned int tipFontSize = 20;

// Number of pickers
constexpr size_t pickerCount = 4;

namespace CustomOvl {
    ComboPicker::ComboPicker() : Overlay() {
        this->setTopLeft(0, this->h() - height);
        this->setBottomRight(this->w(), this->h());
        this->callback = nullptr;

        // Create background panel
        this->rect = new Aether::Rectangle(0, this->h() - height, this->w(), height);
        this->addElement(this->rect);

        // Create title, lines
        this->title = new Aether::Text(this->rect->x() + 72, this->rect->y() + 40, "", titleFontSize);
        this->title->setY(this->title->y() - this->title->h()/2);
        this->addElement(this->title);

        this->topR = new Aether::Rectangle(this->rect->x() + 30, this->rect->y() + 72, this->rect->w() - 60, 1);
        this->addElement(this->topR);
        this->bottomR = new Aether::Rectangle(this->rect->x() + 30, this->rect->y() + this->rect->h() - 72, this->rect->w() - 60, 1);
        this->addElement(this->bottomR);

        // Create OK button + tip
        this->ok = new Aether::BorderButton(0, 0, 160, 60, 3, "OK", 20, [this]() {
            if (this->callback != nullptr) {
                // Gather buttons
                std::vector<Aether::Button> btns;
                for (Picker p : this->pickers) {
                    btns.push_back(p.picker->selectedButton());
                }
                this->callback(btns);
                this->close();
            }
        });
        this->addElement(this->ok);
        this->tip = new Aether::Text(this->rect->x() + 72, this->bottomR->y() + 36, "|", 20);
        this->tip->setY(this->tip->y() - this->tip->h()/2);
        this->addElement(this->tip);

        // Initialize controls
        this->ctrl = nullptr;
        this->setBackLabel("");
        this->setOKLabel("");

        // Create pickers
        for (size_t i = 0; i < pickerCount; i++) {
            Picker p;
            p.picker = new CustomElm::ButtonPicker(0, 0, Aether::Button::NO_BUTTON);
            p.remove = new Aether::BorderButton(0, 0, p.picker->w() * 0.9, 50, 3, "", 18, [p]() {
                p.picker->setSelectedButton(Aether::Button::NO_BUTTON);
            });
            this->addElement(p.picker);
            this->addElement(p.remove);
            this->pickers.push_back(p);
        }

        // Position everything horizontally first
        int totalW = ok->w() + (this->pickers[0].picker->w() * this->pickers.size());
        int gap = (this->w() - 2*padding - totalW)/this->pickers.size();
        for (size_t i = 0; i < this->pickers.size(); i++) {
            Picker p = this->pickers[i];
            p.remove->setX(padding + (gap + p.picker->w())*i);
            p.remove->setX(p.remove->x() + p.picker->w() * 0.05);
            p.picker->setX(padding + (gap + p.picker->w())*i);
        }

        // And now vertically
        int totalH = this->pickers[0].picker->h() + this->pickers[0].remove->h() + 30;
        gap = ((this->bottomR->y() - this->topR->y()) - totalH)/2;
        for (size_t i = 0; i < this->pickers.size(); i++) {
            Picker p = this->pickers[i];
            p.picker->setY(this->topR->y() + gap);
            p.remove->setY(p.picker->y() + p.picker->h() + 30);
        }
        this->ok->setXY(this->w() - padding - this->ok->w(), this->rect->y() + (this->rect->h() - this->ok->h())/2);

        // Override B action to only close if a buttonpicker is not selected
        this->onButtonPress(Aether::Button::B, [this]() {
            for (Picker & p : this->pickers) {
                if (p.picker->selected()) {
                    // If selected we have to send a fake button event (due to how Aether works)
                    SDL_Event event;
                    event.type = SDL_JOYBUTTONDOWN;
                    event.jbutton.which = 69;
                    event.jbutton.button = static_cast<uint8_t>(Aether::Button::B);
                    event.jbutton.state = SDL_PRESSED;

                    Aether::InputEvent e = Aether::InputEvent(event);
                    p.picker->handleEvent(&e);
                    return;
                }
            }
            this->close();
        });
    }

    void ComboPicker::setCombo(const std::vector<Aether::Button> & btns) {
        for (size_t i = 0; i < this->pickers.size(); i++) {
            this->pickers[i].picker->setSelectedButton(i < btns.size() ? btns[i] : Aether::Button::NO_BUTTON);
        }
    }

    void ComboPicker::setCallback(const std::function<void(std::vector<Aether::Button>)> & func) {
        this->callback = func;
    }

    void ComboPicker::setBackLabel(const std::string & s) {
        this->labelBack = s;
        this->removeElement(this->ctrl);
        this->ctrl = new Aether::Controls();
        this->ctrl->addItem(new Aether::ControlItem(Aether::Button::A, this->ok->getString()));
        this->ctrl->addItem(new Aether::ControlItem(Aether::Button::B, this->labelBack));
        this->addElement(this->ctrl);
    }

    void ComboPicker::setOKLabel(const std::string & s) {
        this->ok->setString(s);
        this->removeElement(this->ctrl);
        this->ctrl = new Aether::Controls();
        this->ctrl->addItem(new Aether::ControlItem(Aether::Button::A, this->ok->getString()));
        this->ctrl->addItem(new Aether::ControlItem(Aether::Button::B, this->labelBack));
        this->addElement(this->ctrl);
    }

    void ComboPicker::setRemoveLabel(const std::string & s) {
        for (Picker & p : this->pickers) {
            p.remove->setString(s);
        }
    }

    void ComboPicker::setBackgroundColour(const Aether::Colour & c) {
        this->rect->setColour(c);
    }

    void ComboPicker::setButtonActiveColour(const Aether::Colour & c) {
        for (Picker & p : this->pickers) {
            p.picker->setActiveColour(c);
        }
    }

    void ComboPicker::setButtonInactiveColour(const Aether::Colour & c) {
        for (Picker & p : this->pickers) {
            p.picker->setInactiveColour(c);
        }
    }

    void ComboPicker::setLineColour(const Aether::Colour & c) {
        this->topR->setColour(c);
        this->bottomR->setColour(c);
    }

    void ComboPicker::setMutedTextColour(const Aether::Colour & c) {
        this->tip->setColour(c);
    }

    void ComboPicker::setTextColour(const Aether::Colour & c) {
        this->title->setColour(c);
        this->ctrl->setColour(c);
        this->ok->setBorderColour(c);
        this->ok->setTextColour(c);

        Aether::Colour col = c;
        col.r = c.r * 0.65;
        col.g = c.g * 0.65;
        col.b = c.b * 0.65;
        col.r = c.r * 0.65;
        for (Picker & p : this->pickers) {
            p.remove->setBorderColour(col);
            p.remove->setTextColour(col);
        }
    }

    void ComboPicker::setTitleText(const std::string & s) {
        this->title->setString(s);
        this->title->setY(this->rect->y() + (this->topR->y() - this->rect->y() - this->title->h())/2);
    }

    void ComboPicker::setTipText(const std::string & s) {
        this->tip->setString(s);
    }
};