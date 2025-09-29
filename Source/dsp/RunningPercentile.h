//
// Created by Sol Harter on 09/09/2025.
// P2 Algorihgm implemented from: Raj Jain, Imrich Chlamtac -
// "The P2 algorithm for dynamic calculation of quantiles and histograms without storing observation"
//
// DOI: https://doi.org/10.1145/4372.437

#ifndef AUDIO_NEURO_MOD_RUNNINGPERCENTILE_H
#define AUDIO_NEURO_MOD_RUNNINGPERCENTILE_H

#include <juce_core/juce_core.h>
#include <vector>
#include <algorithm>
#include <cmath>

class RunningPercentile {
public:
    RunningPercentile() : targetPercentile(0.95f), sampleCount(0) {
        initializeP2();
    }

    void addSample(float value) {
        float absValue = std::abs(value);
        sampleCount++;

        if (sampleCount <= 5) { //init with 5 samples
            q[sampleCount - 1] = absValue;
            if (sampleCount == 5) {
                std::sort(q, q + 5);
                for (int i = 0; i < 5; i++) {
                    n[i] = i;
                    np[i] = i;
                }
            }
            return;
        }

        updateP2(absValue);
    }

    float getPercentile(float p = 0.95f) const {
        if (sampleCount == 0) return 1.0f;

        if (std::abs(p - targetPercentile) > 0.001f) {

        }

        if (sampleCount < 5) {
            return 0.1f; // 0.1 default during init
        }

        return q[2]; //
    }

    void clear() {
        sampleCount = 0;
        initializeP2();
    }

private:
    float targetPercentile;
    int sampleCount;

    float q[5];    //Marker heights
    int n[5];      //Marker positions
    float np[5];   //Desired marker positions
    float dn[5];   //Desired position increments

    void initializeP2() {
        dn[0] = 0.0f;
        dn[1] = targetPercentile / 2.0f;
        dn[2] = targetPercentile;         // 95th percentile
        dn[3] = (1.0f + targetPercentile) / 2.0f;
        dn[4] = 1.0f;

        for (int i = 0; i < 5; i++) {
            q[i] = 0.0f;
            n[i] = 0;
            np[i] = 0.0f;
        }
    }

    void updateP2(float newSample) {
        int k = 0;
        if (newSample < q[0]) {
            q[0] = newSample;
            k = 0;
        } else {
            for (int i = 1; i < 5; i++) {
                if (newSample < q[i]) {
                    k = i - 1;
                    break;
                }
                if (i == 4) {
                    q[4] = newSample;
                    k = 3;
                }
            }
        }

        for (int i = k + 1; i < 5; i++) {
            n[i]++;
        }

        for (int i = 0; i < 5; i++) {
            np[i] += dn[i];
        }

        for (int i = 1; i < 4; i++) {
            float d = np[i] - n[i];

            if ((d >= 1.0f && n[i + 1] - n[i] > 1) ||
                (d <= -1.0f && n[i - 1] - n[i] < -1)) {

                int dInt = (d >= 0) ? 1 : -1;

                // Try parabolic formula
                float qs = parabolic(i, dInt);

                // Check if parabolic result is valid
                if (q[i - 1] < qs && qs < q[i + 1]) {
                    q[i] = qs;
                } else {
                    // Use linear formula
                    q[i] = linear(i, dInt);
                }

                n[i] += dInt;
            }
        }
    }

    float parabolic(int i, int d) const {
        float a = (float)d / (n[i + 1] - n[i - 1]);
        float b1 = (n[i] - n[i - 1] + d) * (q[i + 1] - q[i]) / (n[i + 1] - n[i]);
        float b2 = (n[i + 1] - n[i] - d) * (q[i] - q[i - 1]) / (n[i] - n[i - 1]);
        return q[i] + a * (b1 + b2);
    }

    float linear(int i, int d) const {
        return q[i] + d * (q[i + d] - q[i]) / (n[i + d] - n[i]);
    }
};

#endif //AUDIO_NEURO_MOD_RUNNINGPERCENTILE_H