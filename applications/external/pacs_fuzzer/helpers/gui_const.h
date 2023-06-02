#pragma once

// TODO replace it
typedef enum {
    FuzzerMainMenuIndexDefaultValues = 0,
    FuzzerMainMenuIndexLoadFile,
    FuzzerMainMenuIndexLoadFileCustomUids,

    FuzzerMainMenuIndexMax,
} FuzzerMainMenuIndex;

extern const char* fuzzer_attack_names[];