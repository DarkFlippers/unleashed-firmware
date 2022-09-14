#pragma once
#include <gui/view.h>

typedef struct LfRfidTuneView LfRfidTuneView;

LfRfidTuneView* lfrfid_debug_view_tune_alloc();

void lfrfid_debug_view_tune_free(LfRfidTuneView* tune_view);

View* lfrfid_debug_view_tune_get_view(LfRfidTuneView* tune_view);

void lfrfid_debug_view_tune_clean(LfRfidTuneView* tune_view);

bool lfrfid_debug_view_tune_is_dirty(LfRfidTuneView* tune_view);

uint32_t lfrfid_debug_view_tune_get_arr(LfRfidTuneView* tune_view);

uint32_t lfrfid_debug_view_tune_get_ccr(LfRfidTuneView* tune_view);
