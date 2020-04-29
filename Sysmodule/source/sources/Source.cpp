#include "Source.hpp"

Source::Source() {
    this->channels_ = 0;
    this->sampleRate_ = 0;
    this->decodedSamples_ = 0;
    this->totalSamples_ = 1;    // avoid NaN
    this->valid_ = true;
}

bool Source::done() {
    return (this->decodedSamples_ == this->totalSamples_);
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

double Source::position() {
    return (this->decodedSamples_/(double)this->totalSamples_);
}

Source::~Source() {

}