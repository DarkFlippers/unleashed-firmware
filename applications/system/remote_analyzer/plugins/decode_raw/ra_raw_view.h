/**
 * @file ra_raw_view.h
 * @brief The screen a capture is read behind: which file, and how far through it we are.
 */
#pragma once

#include <gui/view.h>

typedef struct RaRawView RaRawView;

RaRawView* ra_raw_view_alloc(void);
void ra_raw_view_free(RaRawView* instance);
View* ra_raw_view_get_view(RaRawView* instance);

/** Reading @p name, @p percent of the way through it. */
void ra_raw_view_set_progress(RaRawView* instance, const char* name, uint8_t percent);

/** Done, with nothing to list, or not started at all: @p headline over @p detail, no bar. */
void ra_raw_view_set_done(RaRawView* instance, const char* headline, const char* detail);
