#include "ProgressBar.hpp"

namespace Aether {
    ProgressBar::ProgressBar(int x, int y, int w) : BaseProgress(x, y, w, 16) {
        this->boxTex = new Box(this->x(), this->y(), this->w(), this->h(), 1);
        this->addElement(this->boxTex);
        this->progressTex = new Rectangle(this->x() + 3, this->y() + 3, this->w() - 6, this->h() - 6);
        this->addElement(this->progressTex);
    }

    void ProgressBar::redrawBar() {
        this->boxTex->setBoxSize(this->w(), this->h());
        this->progressTex->setRectSize(this->w(), this->h());
    }

    void ProgressBar::setValue(float f) {
        float old = this->value();
        BaseProgress::setValue(f);

        if (old != this->value()) {
            this->progressTex->setW(this->progressTex->texW() * (this->value()/100.0));
            this->progressTex->setMask(0, 0, this->progressTex->texW() * (this->value()/100.0), this->progressTex->texH());
        }
    }

    void ProgressBar::setW(int w) {
        BaseProgress::setW(w);
        this->redrawBar();
    }

    void ProgressBar::setH(int h) {
        BaseProgress::setH(h);
        this->redrawBar();
    }

    Colour ProgressBar::getColour() {
        return this->boxTex->getColour();
    }

    void ProgressBar::setColour(Colour c) {
        this->boxTex->setColour(c);
        this->progressTex->setColour(c);
    }
};