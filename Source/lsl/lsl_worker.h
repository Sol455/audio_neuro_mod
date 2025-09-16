// Created by Sol Harter on 28/08/2025.

#ifndef AUDIO_NEURO_MOD_LSL_WORKER_H
#define AUDIO_NEURO_MOD_LSL_WORKER_H

#include <juce_core/juce_core.h>
#include <lsl_cpp.h>
#include <atomic>
#include <vector>
#include "EegFIFO.h"
#include "timestampMapper.h"

class LslWorker : private juce::Thread
{
public:
    explicit LslWorker (EegFIFO& dst, timestampMapper& mapper)
        : juce::Thread ("LSL Worker"), ring (dst), stampMapper(mapper) {}

    void setInlet   (lsl::stream_inlet* p) { inlet.store (p, std::memory_order_release); }
    void setChannel (const int idx)              { channel.store (idx, std::memory_order_relaxed); std::cout << "LslWorker: Channel changed to "<< idx << std::endl; }
    int  getChannel () const               { return channel.load (std::memory_order_relaxed); }

    void startWorker() { if (! isThreadRunning()) startThread (Priority::high); }
    void stopWorker()  { signalThreadShouldExit(); stopThread (1500); }

private:
    void run() override
    {
        std::vector<std::vector<float>> chunk_data;
        std::vector<double> chunk_timestamps;
        int chCount = 0;

        while (! threadShouldExit())
        {
            auto* in = inlet.load (std::memory_order_acquire);
            if (! in) { juce::Thread::yield(); continue; }

            if (chCount <= 0)
            {
                try { chCount = in->info().channel_count(); }
                catch (...) { juce::Thread::sleep (5); continue; }

                if (chCount <= 0) { juce::Thread::sleep (5); continue; }
            }

            const int wantCh = juce::jlimit (0, chCount - 1, channel.load (std::memory_order_relaxed));

            bool got_data = false;
            try {
                got_data = in->pull_chunk(chunk_data, chunk_timestamps);
            } catch (...) {
                juce::Thread::sleep(1); // time out if no data ready
                continue;
            }

            if (got_data && !chunk_data.empty()) {
                for (size_t i = 0; i < chunk_data.size(); ++i) {
                    if (chunk_data[i].size() > wantCh) {
                        int64_t hostSampleIndex = stampMapper.toHostSample(chunk_timestamps[i]);
                        EegSample sample{chunk_data[i][wantCh], hostSampleIndex};

                        if (!ring.addSample(sample)) {
                            std::cout << "LSL outlet buffer full" << std::endl;
                        }
                    }
                }
            }
        }
    }

    EegFIFO&                        ring;
    timestampMapper&                stampMapper;
    std::atomic<lsl::stream_inlet*> inlet   { nullptr }; // non-owning
    std::atomic<int>                channel { 0 };
};

#endif // AUDIO_NEURO_MOD_LSL_WORKER_H

