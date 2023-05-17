#pragma once
#include <gui/view.h>

typedef enum {
    LfRfidReadAsk,
    LfRfidReadPsk,
    LfRfidReadHitag,
    LfRfidReadAskOnly,
    LfRfidReadPskOnly,
    LfRfidReadRtfOnly,
} LfRfidReadViewMode;

typedef enum {
    LfRfidReadScanning,
    LfRfidReadTagDetected,
} LfRfidReadViewState;

typedef struct LfRfidReadView LfRfidReadView;

LfRfidReadView* lfrfid_view_read_alloc();

void lfrfid_view_read_free(LfRfidReadView* read_view);

View* lfrfid_view_read_get_view(LfRfidReadView* read_view);

void lfrfid_view_read_set_read_mode(LfRfidReadView* read_view, LfRfidReadViewMode mode);

void lfrfid_view_read_set_read_state(LfRfidReadView* read_view, LfRfidReadViewState state);