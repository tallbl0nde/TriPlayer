#include "RoundProgressBar.hpp"

namespace Aether {
    RoundProgressBar::RoundProgressBar(int x, int y, int w, int h) : BaseProgress(x, y, w, h) {
        // Render textures
        this->backTex = new Rectangle(this->x(), this->y(), this->w(), this->h(), (this->h()/2) - 1);
        this->addElement(this->backTex);
        this->progressTex = new Rectangle(this->x(), this->y(), this->w(), this->h(), (this->h()/2) - 1);
        this->addElement(this->progressTex);
    }

    void RoundProgressBar::redrawBar() {
        this->backTex->setRectSize(this->w(), this->h());
        this->progressTex->setRectSize(this->w(), this->h());
    }

    void RoundProgressBar::setValue(float f) {
        float old = this->value();
        BaseProgress::setValue(f);

        if (old != this->value()) {
            this->progressTex->setW(this->progressTex->texW() * (this->value()/100.0));
            this->progressTex->setMask(0, 0, this->progressTex->texW() * (this->value()/100.0), this->progressTex->texH());
        }
    }

    void RoundProgressBar::setW(int w) {
        BaseProgress::setW(w);
        this->redrawBar();
    }

    void RoundProgressBar::setH(int h) {
        BaseProgress::setH(h);
        this->redrawBar();
    }

    Colour RoundProgressBar::getBackgroundColour() {
        return this->backTex->getColour();
    }

    void RoundProgressBar::setBackgroundColour(Colour c) {
        this->backTex->setColour(c);
    }

    Colour RoundProgressBar::getForegroundColour() {
        return this->progressTex->getColour();
    }

    void RoundProgressBar::setForegroundColour(Colour c) {
        this->progressTex->setColour(c);
    }

};