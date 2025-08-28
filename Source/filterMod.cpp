//
// Created by Sol Harter on 28/08/2025.
//

#include "filterMod.h"

void filterMod::prepare (double fs, double centreHz, double Q)
{
    fs_ = fs;
    setBand(centreHz, Q);
    bp_.reset();
}

void filterMod::setBand(double centreHz, double Q) {
    jassert (fs_ > 0.0);
    auto coeff = juce::dsp::IIR::Coefficients<float>::makeBandPass (fs_, centreHz, Q);
    bp_.coefficients = coeff;
}
