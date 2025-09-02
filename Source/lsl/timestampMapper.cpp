#include "timestampMapper.h"


void timestampMapper::prepare(double sampleRate) {
    a.store(sampleRate);

    //perform initial calibration
    double currentLslTime = lsl::local_clock();
    int64_t currentHostSample = 0;

    double initialB = currentHostSample - sampleRate * currentLslTime;
    b.store(initialB);

    lastLslTime.store(0.0);
    lastHostSample.store(0);
}


void timestampMapper::calibrate(double lslTime, int64_t hostSample) {
    double prevLslTime = lastLslTime.exchange(lslTime);
    int64_t prevHostSample = lastHostSample.exchange(hostSample);

    // Skip first calibration (
    if (prevLslTime == 0.0) return;

    double deltaTime = lslTime - prevLslTime;
    int64_t deltaSamples = hostSample - prevHostSample;

    if (deltaTime > 0.1) {  // Only update if enough time passed
        double newA = static_cast<double>(deltaSamples) / deltaTime;
        double newB = static_cast<double>(hostSample) - newA * lslTime;

        // Smooth
        a.store(a.load() * 0.9 + newA * 0.1);
        b.store(b.load() * 0.9 + newB * 0.1);
        }
    }

// Convert LSL timestamp to host sample
int64_t timestampMapper::toHostSample(double lslTime) {
    return static_cast<int64_t>(a.load() * lslTime + b.load());
}


