//
// Created by Sol Harter on 15/10/2025.
//

#ifndef AUDIO_NEURO_MOD_MACROS_H
#define AUDIO_NEURO_MOD_MACROS_H

// Defines the fixed delay (in milliseconds) before processing incoming LSL samples.
// This value should correspond to the EEG amplifier’s frame size and sampling rate.
// Larger frame sizes and lower sampling rates require a longer jitter buffer to ensure stable timing.
// 40 ms is a conservative default for most configurations, with high sample rates and single sample frame sizes this could be lowered significantly.
#define LSL_JITTER_BUF_DELAY_MS 40

// Sets the number of samples by which the CFIR filter output is delayed.
// Lower values reduce latency but result in less accurate phase approximation.
#define CFIR_DELAY_SAMPLES 50

//Output Midi Channel
#define MIDI_OUTPUT_CHANNEL 1

//Output Control Change (CC) number
#define MIDI_OUTPUT_CC 74
#endif //AUDIO_NEURO_MOD_MACROS_H