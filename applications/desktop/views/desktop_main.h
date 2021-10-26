#pragma once

#include <gui/gui_i.h>
#include <gui/view.h>
#include <gui/canvas.h>
#include <gui/elements.h>
#include <furi.h>

typedef enum {
    DesktopMainEventOpenLockMenu,
    DesktopMainEventOpenArchive,
    DesktopMainEventOpenFavorite,
    DesktopMainEventOpenMenu,
    DesktopMainEventOpenDebug,
    DesktopMainEventUnlocked,
} DesktopMainEvent;

typedef struct DesktopMainView DesktopMainView;

typedef void (*DesktopMainViewCallback)(DesktopMainEvent event, void* context);

struct DesktopMainView {
    View* view;
    DesktopMainViewCallback callback;
    void* context;
};

typedef struct {
    IconAnimation* animation;
    uint8_t scene_num;
    uint32_t hint_expire_at;
} DesktopMainViewModel;

void desktop_main_set_callback(
    DesktopMainView* main_view,
    DesktopMainViewCallback callback,
    void* context);

View* desktop_main_get_view(DesktopMainView* main_view);
DesktopMainView* desktop_main_alloc();
void desktop_main_free(DesktopMainView* main_view);
void desktop_main_switch_dolphin_animation(DesktopMainView* main_view);
void desktop_main_unlocked(DesktopMainView* main_view);
void desktop_main_reset_hint(DesktopMainView* main_view);
