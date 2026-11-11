/**
 * @file ra_read_view.h
 * @brief The Read screen: the last decoded signal, and nothing behind it.
 *
 * No history and no list - a new signal replaces what is on screen. Left opens the config,
 * right clears the screen so the next press lands on an empty one.
 */
#pragma once

#include <gui/view.h>

typedef struct RaReadView RaReadView;

typedef enum {
    RaReadViewEventConfig,
    RaReadViewEventReset,
} RaReadViewEvent;

/** Kept separate from SubGhzProtocolType so the view does not pull in lib/subghz. */
typedef enum {
    RaReadViewKindNone = 0, // nothing decoded yet, no badge
    RaReadViewKindUnknown,
    RaReadViewKindStatic,
    RaReadViewKindDynamic,
} RaReadViewKind;

typedef void (*RaReadViewCallback)(RaReadViewEvent event, void* context);

RaReadView* ra_read_view_alloc(void);

void ra_read_view_free(RaReadView* instance);

View* ra_read_view_get_view(RaReadView* instance);

void ra_read_view_set_callback(RaReadView* instance, RaReadViewCallback callback, void* context);

/** Show a decoded signal. @p text is the decoder's own description, CRLF separated.
 * @p kind drives the static/dynamic badge in the header. */
void ra_read_view_set_signal(RaReadView* instance, const char* text, RaReadViewKind kind);

/** Receiver strength for the header bar, in dBm. */
void ra_read_view_set_rssi(RaReadView* instance, float rssi);

/** A headline and a line under it, centred, in place of a reading: what the modulation sweep
 * is doing, or how a file decode is getting on. Both NULL puts the screen back to normal. */
void ra_read_view_set_status(RaReadView* instance, const char* status, const char* detail);

/** Back to an empty screen. */
void ra_read_view_clear(RaReadView* instance);

/** Update the frequency and modulation shown in the header. */
void ra_read_view_set_radio(RaReadView* instance, const char* frequency, const char* modulation);
