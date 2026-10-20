/**
 * @file t5577_work_view.h
 * @brief What a read or a write is doing, block by block.
 */
#pragma once

#include <gui/view.h>

typedef struct T5577WorkView T5577WorkView;

T5577WorkView* t5577_work_view_alloc(void);

void t5577_work_view_free(T5577WorkView* instance);

View* t5577_work_view_get_view(T5577WorkView* instance);

/** What the screen is for, and how many blocks the bar is counting towards. */
void t5577_work_view_start(T5577WorkView* instance, const char* title, uint16_t total);

/** Where it has got to: a line of its own, and a block count for the bar. */
void t5577_work_view_set_status(T5577WorkView* instance, const char* status, uint16_t done);
