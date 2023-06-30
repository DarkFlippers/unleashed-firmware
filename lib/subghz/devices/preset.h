#pragma once

/** Radio Presets */
typedef enum {
    FuriHalSubGhzPresetIDLE, /**< default configuration */
    FuriHalSubGhzPresetOok270Async, /**< OOK, bandwidth 270kHz, asynchronous */
    FuriHalSubGhzPresetOok650Async, /**< OOK, bandwidth 650kHz, asynchronous */
    FuriHalSubGhzPreset2FSKDev238Async, /**< FM, deviation 2.380371 kHz, asynchronous */
    FuriHalSubGhzPreset2FSKDev476Async, /**< FM, deviation 47.60742 kHz, asynchronous */
    FuriHalSubGhzPresetMSK99_97KbAsync, /**< MSK, deviation 47.60742 kHz, 99.97Kb/s, asynchronous */
    FuriHalSubGhzPresetGFSK9_99KbAsync, /**< GFSK, deviation 19.042969 kHz, 9.996Kb/s, asynchronous */
    FuriHalSubGhzPresetCustom, /**Custom Preset*/
} FuriHalSubGhzPreset;
