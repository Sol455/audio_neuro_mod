//
// Created by jake on 16/09/2025.
//

#ifndef AUDIO_NEURO_MOD_SIMPLEPROFILER_H
#define AUDIO_NEURO_MOD_SIMPLEPROFILER_H
#include <chrono>


class SimpleProfiler {
public:
    void start() {
        startTime = std::chrono::high_resolution_clock::now();
    }

    void stop() {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);
        lastTime = duration.count() / 1000000.0; // Convert to milliseconds

        totalTime += lastTime;
        count++;
    }

    double getTime() const {
        return lastTime;
    }

    double getAverage() const {
        return count > 0 ? totalTime / count : 0.0;
    }

    void reset() {
        totalTime = 0.0;
        count = 0;
        lastTime = 0.0;
    }

private:
    std::chrono::high_resolution_clock::time_point startTime;
    double lastTime = 0.0;
    double totalTime = 0.0;
    int count = 0;
};

#endif //AUDIO_NEURO_MOD_SIMPLEPROFILER_H