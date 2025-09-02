//
// Created by Sol Harter on 01/09/2025.
//

#ifndef AUDIO_NEURO_MOD_EEGTYPES_H
#define AUDIO_NEURO_MOD_EEGTYPES_H

#pragma once
#include <cstdint>

struct EegSample {
    float value;
    int64_t stamp;
};

#endif //AUDIO_NEURO_MOD_EEGTYPES_H