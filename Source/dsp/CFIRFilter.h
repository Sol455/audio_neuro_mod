//
// Created by Sol Harter on 09/09/2025.
//

#ifndef AUDIO_NEURO_MOD_CFIRFILTER_H
#define AUDIO_NEURO_MOD_CFIRFILTER_H

#include "juce_audio_basics/juce_audio_basics.h"
#include <complex>
#include <vector>
/**
 * Direct mirror of CFIR filter: https://pubmed.ncbi.nlm.nih.gov/32289760/
 */
class CFIRFilter
{
public:
    CFIRFilter() = default;
    ~CFIRFilter() = default;

    void prepare(double bandLow, double bandHigh, double fs,
                 int delay = 0, int nTaps = 500, int nFFT = 2000);

    // Process a sample
    std::complex<float> processSample(float input);

    // Reset filters state
    void reset();

private:
    double bandLow = 8.0;
    double bandHigh = 12.0;
    double fs = 1000.0;
    int delay = 0;
    int nTaps = 500;
    int nFFT = 2000;

    std::vector<std::complex<float>> filterCoeffs;  // b coefficients
    std::vector<float> delayLine;                   // Filter delay line/ state

    void designFilter();
};

#endif //AUDIO_NEURO_MOD_CFIRFILTER_H