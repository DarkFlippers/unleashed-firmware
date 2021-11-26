#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <gui/icon.h>

typedef struct DesktopAnimation DesktopAnimation;

typedef struct ActiveAnimation ActiveAnimation;
typedef struct BasicAnimation BasicAnimation;

typedef enum {
    DesktopAnimationStateBasic,
    DesktopAnimationStateActive,
    DesktopAnimationStateLevelUpIsPending,
    DesktopAnimationStateSDEmpty,
    DesktopAnimationStateSDEmptyURL,
    DesktopAnimationStateSDCorrupted,
} DesktopAnimationState;

struct BasicAnimation {
    const Icon* icon;
    uint16_t duration; // sec
    uint16_t active_cooldown;
    uint8_t weight;
    bool black_status_bar;
    uint16_t butthurt_level_mask;
};

struct ActiveAnimation {
    const Icon* icon;
    bool black_status_bar;
    uint16_t duration; // sec
};

typedef struct {
    const BasicAnimation* basic;
    const ActiveAnimation* active;
} PairedAnimation;

typedef void (*AnimationChangedCallback)(void*);

DesktopAnimation* desktop_animation_alloc(void);
void desktop_animation_free(DesktopAnimation*);
void desktop_animation_activate(DesktopAnimation* instance);
void desktop_animation_set_animation_changed_callback(
    DesktopAnimation* instance,
    AnimationChangedCallback callback,
    void* context);

DesktopAnimationState desktop_animation_handle_right(DesktopAnimation* animation);

void desktop_animation_start_oneshot_levelup(DesktopAnimation* animation);

const Icon*
    desktop_animation_get_animation(DesktopAnimation* animation, bool* status_bar_background_black);
const Icon* desktop_animation_get_oneshot_frame(DesktopAnimation* animation);

void desktop_start_new_idle_animation(DesktopAnimation* animation);
