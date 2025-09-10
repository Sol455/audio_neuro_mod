//
// Created by Sol Harter on 09/09/2025.
//

#include "CFIRFilter.h"
#include <numeric>
#include <algorithm>

void CFIRFilter::prepare(double bandLow, double bandHigh, double fs,
                        int delay, int nTaps, int nFFT)
{
    this->bandLow = bandLow;
    this->bandHigh = bandHigh;
    this->fs = fs;
    this->delay = delay;
    this->nTaps = nTaps;
    this->nFFT = nFFT;

    // Design the filter coefficients
    designFilter();

    // Initialize filter delay line
    delayLine.assign(nTaps - 1, 0.0f);
}

std::complex<float> CFIRFilter::processSample(float input)
{
    // Shift delay line:
    for (int i = delayLine.size() - 1; i > 0; --i)
    {
        delayLine[i] = delayLine[i - 1];
    }
    delayLine[0] = input;

    // FIR convolution:
    std::complex<float> output = filterCoeffs[0] * input;

    for (int k = 0; k < nTaps - 1; ++k)
    {
        output += filterCoeffs[k + 1] * delayLine[k];
    }

    return output;
}

void CFIRFilter::reset()
{
    std::fill(delayLine.begin(), delayLine.end(), 0.0f);
}

void CFIRFilter::designFilter()
{

    std::vector<int> w(nFFT);
    std::iota(w.begin(), w.end(), 0);

    std::vector<std::complex<float>> H(nFFT);
    for (int k = 0; k < nFFT; ++k)
    {
        double phase = -2.0 * juce::MathConstants<double>::pi * k * delay / nFFT;
        H[k] = std::complex<float>(
            2.0f * std::cos(phase),
            2.0f * std::sin(phase)
        );
    }

    for (int k = 0; k < nFFT; ++k)
    {
        double freq = (k * fs) / nFFT;

        if (freq < bandLow || freq > bandHigh)
        {
            H[k] = std::complex<float>(0.0f, 0.0f);
        }
    }

    filterCoeffs.resize(nTaps);

    for (int n = 0; n < nTaps; ++n)
    {
        std::complex<float> coeff(0.0f, 0.0f);

        for (int k = 0; k < nFFT; ++k)
        {
            double phase = 2.0 * juce::MathConstants<double>::pi * k * n / nFFT;
            std::complex<float> fConjugate(std::cos(phase), std::sin(phase));
            coeff += fConjugate * H[k];
        }
        filterCoeffs[n] = coeff / static_cast<float>(nFFT);
    }
}