#pragma once

#include <gui/view.h>

typedef struct InfraredMoveView InfraredMoveView;

typedef void (*InfraredMoveCallback)(uint32_t index_old, uint32_t index_new, void* context);

typedef const char* (*InfraredMoveGetItemCallback)(uint32_t index, void* context);

InfraredMoveView* infrared_move_view_alloc(void);

void infrared_move_view_free(InfraredMoveView* debug_view);

View* infrared_move_view_get_view(InfraredMoveView* debug_view);

void infrared_move_view_set_callback(InfraredMoveView* move_view, InfraredMoveCallback callback);

void infrared_move_view_list_init(
    InfraredMoveView* move_view,
    uint32_t item_count,
    InfraredMoveGetItemCallback load_cb,
    void* context);

void infrared_move_view_list_update(InfraredMoveView* move_view);