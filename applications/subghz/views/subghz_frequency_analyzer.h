#pragma once

#include <gui/view.h>

typedef enum {
    SubghzFrequencyAnalyzerEventOnlyRx,
} SubghzFrequencyAnalyzerEvent;

typedef struct SubghzFrequencyAnalyzer SubghzFrequencyAnalyzer;

typedef void (*SubghzFrequencyAnalyzerCallback)(SubghzFrequencyAnalyzerEvent event, void* context);

void subghz_frequency_analyzer_set_callback(
    SubghzFrequencyAnalyzer* subghz_frequency_analyzer,
    SubghzFrequencyAnalyzerCallback callback,
    void* context);

SubghzFrequencyAnalyzer* subghz_frequency_analyzer_alloc();

void subghz_frequency_analyzer_free(SubghzFrequencyAnalyzer* subghz_static);

View* subghz_frequency_analyzer_get_view(SubghzFrequencyAnalyzer* subghz_static);
