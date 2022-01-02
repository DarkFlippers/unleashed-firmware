#pragma once

#include <gui/gui_i.h>
#include <gui/view.h>
#include <gui/canvas.h>
#include <gui/elements.h>
#include <furi.h>
#include "desktop_events.h"

#define UNLOCK_RST_TIMEOUT 300
#define UNLOCK_CNT 2 // 3 actually

#define DOOR_L_POS -57
#define DOOR_L_POS_MAX 0
#define DOOR_R_POS 115
#define DOOR_R_POS_MIN 60

typedef enum {
    DesktopLockedWithPin,
    DesktopLockedNoPin,
} DesktopLockedSceneState;

typedef struct DesktopLockedView DesktopLockedView;

typedef void (*DesktopLockedViewCallback)(DesktopEvent event, void* context);

struct DesktopLockedView {
    View* view;
    DesktopLockedViewCallback callback;
    void* context;

    osTimerId_t timer;
    uint8_t lock_count;
    uint32_t lock_lastpress;
};

typedef struct {
    IconAnimation* animation;
    uint32_t hint_expire_at;

    bool status_bar_background_black;
    uint8_t scene_num;
    int8_t door_left_x;
    int8_t door_right_x;
    bool animation_seq_end;

    bool pin_lock;
} DesktopLockedViewModel;

void desktop_locked_set_callback(
    DesktopLockedView* locked_view,
    DesktopLockedViewCallback callback,
    void* context);

void desktop_locked_update_hint_timeout(DesktopLockedView* locked_view);
void desktop_locked_reset_counter(DesktopLockedView* locked_view);
void desktop_locked_reset_door_pos(DesktopLockedView* locked_view);
void desktop_locked_manage_redraw(DesktopLockedView* locked_view);

View* desktop_locked_get_view(DesktopLockedView* locked_view);
DesktopLockedView* desktop_locked_alloc();
void desktop_locked_free(DesktopLockedView* locked_view);
void desktop_locked_with_pin(DesktopLockedView* lock_menu, bool locked);
