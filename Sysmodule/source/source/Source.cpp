#include "source/Source.hpp"

namespace Source {
    Source::Source() {
        this->channels_ = 0;
        this->done_ = false;
        this->sampleRate_ = 0;
        this->totalSamples_ = 1;    // avoid NaN
        this->valid_ = true;
    }

    bool Source::done() {
        return this->done_;
    }

    bool Source::valid() {
        return this->valid_;
    }

    int Source::channels() {
        return this->channels_;
    }

    long Source::sampleRate() {
        return this->sampleRate_;
    }

    int Source::totalSamples() {
        return this->totalSamples_;
    }

    Source::~Source() {

    }
};