/**
 * @file t5577_listen_view.h
 * @brief What the coil is picking up, while it picks it up.
 *
 * The work view's bar is the wrong instrument here and was tried twice. A capture has no total
 * to count towards - one press of a cloner is a dozen commands and every answer it holds - so a
 * bar either implies the user is falling short of something or counts down a window nobody asked
 * about. What somebody holding a Flipper against a cloner wants to know is whether it is hearing
 * anything, and how the coupling changes as they move it.
 *
 * So this draws a trace instead: one bar per tick, tall where gaps arrived and flat where none
 * did, scrolling left. It shows activity, never progress.
 */
#pragma once

#include <gui/view.h>

/** Ticks of history the trace holds.
 *
 * At two pixels each they fill the screen's width, and at the listening scene's tick that is
 * about twelve seconds - long enough that a whole write run stays on screen after it has
 * finished, rather than scrolling away before the user can look up.
 */
#define T5577_LISTEN_VIEW_SAMPLES 60

typedef struct T5577ListenView T5577ListenView;

T5577ListenView* t5577_listen_view_alloc(void);

void t5577_listen_view_free(T5577ListenView* instance);

View* t5577_listen_view_get_view(T5577ListenView* instance);

/** Name what the screen is for, and clear the trace. */
void t5577_listen_view_start(T5577ListenView* instance, const char* title);

/** What it is doing, in the line under the title. */
void t5577_listen_view_set_status(T5577ListenView* instance, const char* status);

/** Add one tick to the trace, as the two numbers the caller measured.
 *
 * `value` against `full` rather than a level in pixels: how many gaps saturate a tick is a radio
 * measurement and belongs to whoever counted them, while how tall a saturated tick is drawn is
 * this view's. Taking a pre-scaled level put both halves in the caller.
 */
void t5577_listen_view_push(T5577ListenView* instance, uint32_t value, uint32_t full);
