#pragma once

#include <gui/view.h>
#include "../helpers/subghz_custom_event.h"
#include "../helpers/subghz_txrx.h"

typedef enum {
    SubGHzFrequencyAnalyzerFeedbackLevelAll,
    SubGHzFrequencyAnalyzerFeedbackLevelVibro,
    SubGHzFrequencyAnalyzerFeedbackLevelMute
} SubGHzFrequencyAnalyzerFeedbackLevel;

typedef struct SubGhzFrequencyAnalyzer SubGhzFrequencyAnalyzer;

typedef void (*SubGhzFrequencyAnalyzerCallback)(SubGhzCustomEvent event, void* context);

void subghz_frequency_analyzer_set_callback(
    SubGhzFrequencyAnalyzer* subghz_frequency_analyzer,
    SubGhzFrequencyAnalyzerCallback callback,
    void* context);

SubGhzFrequencyAnalyzer* subghz_frequency_analyzer_alloc(SubGhzTxRx* txrx);

void subghz_frequency_analyzer_free(SubGhzFrequencyAnalyzer* subghz_static);

View* subghz_frequency_analyzer_get_view(SubGhzFrequencyAnalyzer* subghz_static);

uint32_t subghz_frequency_analyzer_get_frequency_to_save(SubGhzFrequencyAnalyzer* instance);

SubGHzFrequencyAnalyzerFeedbackLevel subghz_frequency_analyzer_feedback_level(
    SubGhzFrequencyAnalyzer* instance,
    SubGHzFrequencyAnalyzerFeedbackLevel level,
    bool update);

float subghz_frequency_analyzer_get_trigger_level(SubGhzFrequencyAnalyzer* instance);