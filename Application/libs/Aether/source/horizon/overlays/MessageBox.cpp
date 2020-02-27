#include "MessageBox.hpp"

// Default dimensions
#define WIDTH 770
#define HEIGHT 400

// Button height
#define BUTTON_HEIGHT 70

namespace Aether {
    MessageBox::MessageBox() {
        this->rect = new Rectangle(640 - WIDTH/2, 360 - HEIGHT/2, WIDTH, HEIGHT, 8);
        this->addElement(this->rect);

        this->body = nullptr;
        this->left = nullptr;
        this->right = nullptr;
        this->top = nullptr;
        this->topH = new Rectangle(this->rect->x(), this->rect->y(), this->rect->w(), 2);
        this->bottomH = new Rectangle(this->rect->x(), this->rect->y(), this->rect->w(), 2);
        this->vertH = new Rectangle(this->rect->x() + this->rect->w()/2, this->rect->y() + this->rect->h() - BUTTON_HEIGHT, 2, BUTTON_HEIGHT);
        this->topH->setHidden(true);
        this->vertH->setHidden(true);
        this->addElement(this->topH);
        this->addElement(this->bottomH);
        this->addElement(this->vertH);

        // Pressing b just closes the box
        this->onButtonPress(Button::B, [this](){
            this->close(true);
        });
    }

    void MessageBox::repositionButtons() {
        bool twoRows = (this->top != nullptr && (this->left != nullptr || this->right != nullptr));
        this->bottomH->setY(this->rect->y() + this->rect->h() - BUTTON_HEIGHT - 2);
        if (this->top != nullptr) {
            if (twoRows) {
                this->top->setXY(this->rect->x(), this->rect->y() + this->rect->h() - 2*BUTTON_HEIGHT - 1);
                this->topH->setY(this->rect->y() + this->rect->h() - 2*BUTTON_HEIGHT - 3);
            } else {
                this->top->setXY(this->rect->x(), this->rect->y() + this->rect->h() - BUTTON_HEIGHT);
                this->topH->setY(this->rect->y() + this->rect->h() - BUTTON_HEIGHT - 2);
            }
        }

        if (this->left != nullptr) {
            this->left->setXY(this->rect->x(), this->rect->y() + this->rect->h() - BUTTON_HEIGHT);
            this->vertH->setX(this->rect->x() + this->rect->w()/2 - 1);
        }

        if (this->right != nullptr) {
            this->right->setXY(this->rect->x() + this->rect->w()/2 + 1, this->rect->y() + this->rect->h() - BUTTON_HEIGHT);
            this->vertH->setX(this->rect->x() + this->rect->w()/2 - 1);
        }
    }

    void MessageBox::resizeElements() {
        this->topH->setRectSize(this->rect->w() - 1, 2);
        this->bottomH->setRectSize(this->rect->w() - 1, 2);
        if (this->left != nullptr) {
            this->left->setW(this->rect->w()/2);
        }
        if (this->right != nullptr) {
            this->right->setW(this->rect->w()/2 - 1);
        }
        if (this->top != nullptr) {
            this->top->setW(this->rect->w());
        }
    }

    void MessageBox::setLineColour(Colour c) {
        this->topH->setColour(c);
        this->bottomH->setColour(c);
        this->vertH->setColour(c);
    }

    void MessageBox::setRectangleColour(Colour c) {
        this->rect->setColour(c);
    }

    void MessageBox::setTextColour(Colour c) {
        if (this->left != nullptr) {
            this->left->setTextColour(c);
        }
        if (this->right != nullptr) {
            this->right->setTextColour(c);
        }
        if (this->top != nullptr) {
            this->top->setTextColour(c);
        }
    }

    void MessageBox::addLeftButton(std::string s, std::function<void()> f) {
        this->left = new BorderButton(0, 0, this->rect->w()/2, BUTTON_HEIGHT, 2, s, 26, f);
        this->left->setBorderColour(Colour{255, 255, 255, 0});
        this->addElement(this->left);
        this->vertH->setHidden(false);
        this->repositionButtons();
    }

    void MessageBox::addRightButton(std::string s, std::function<void()> f) {
        this->right = new BorderButton(0, 0, this->rect->w()/2 - 1, BUTTON_HEIGHT, 2, s, 26, f);
        this->right->setBorderColour(Colour{255, 255, 255, 0});
        this->addElement(this->right);
        this->vertH->setHidden(false);
        this->repositionButtons();
    }

    void MessageBox::addTopButton(std::string s, std::function<void()> f) {
        this->top = new BorderButton(0, 0, this->rect->w(), BUTTON_HEIGHT, 2, s, 26, f);
        this->top->setBorderColour(Colour{255, 255, 255, 0});
        this->addElement(this->top);
        this->topH->setHidden(false);
        this->repositionButtons();
    }

    void MessageBox::getBodySize(int * w, int * h) {
        *w = this->rect->w();
        *h = this->rect->h();
        if (this->top != nullptr) {
            *h -= BUTTON_HEIGHT;
        }
        if (this->left != nullptr || this->right != nullptr) {
            *h -= BUTTON_HEIGHT;
        }
    }

    void MessageBox::setBodySize(int w, int h) {
        if (this->top != nullptr) {
            h += BUTTON_HEIGHT;
        }
        if (this->left != nullptr || this->right != nullptr) {
            h += BUTTON_HEIGHT;
        }
        this->rect->setRectSize(w, h);
        this->rect->setXY(640 - this->rect->w()/2, 360 - this->rect->h()/2);
        if (this->body != nullptr) {
            this->body->setXY(this->rect->x(), this->rect->y());
        }
        this->topH->setX(this->rect->x());
        this->bottomH->setX(this->rect->x());
        this->vertH->setY(this->rect->y() + this->rect->h() - BUTTON_HEIGHT);
        this->resizeElements();
        this->repositionButtons();
    }

    void MessageBox::emptyBody() {
        if (this->body != nullptr) {
            this->removeElement(this->body);
        }
        this->body = nullptr;
    }

    void MessageBox::setBody(Element * e) {
        if (this->body != nullptr) {
            return;
        }

        this->body = e;
        this->body->setXY(this->rect->x(), this->rect->y());
        this->addElement(this->body);
    }
};