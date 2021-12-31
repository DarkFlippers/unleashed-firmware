#include "assets_icons.h"
#include "cmsis_os2.h"
#include "desktop/desktop.h"
#include "desktop_i.h"
#include <dolphin/dolphin.h>
#include <furi/pubsub.h>
#include <furi/record.h>
#include "portmacro.h"
#include "storage/filesystem-api-defines.h"
#include "storage/storage.h"
#include <stdint.h>
#include <power/power_service/power.h>
#include "helpers/desktop_animation.h"

static void desktop_lock_icon_callback(Canvas* canvas, void* context) {
    furi_assert(canvas);
    canvas_draw_icon(canvas, 0, 0, &I_Lock_8x8);
}

bool desktop_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    Desktop* desktop = (Desktop*)context;
    return scene_manager_handle_custom_event(desktop->scene_manager, event);
}

bool desktop_back_event_callback(void* context) {
    furi_assert(context);
    Desktop* desktop = (Desktop*)context;
    return scene_manager_handle_back_event(desktop->scene_manager);
}

Desktop* desktop_alloc() {
    Desktop* desktop = furi_alloc(sizeof(Desktop));

    desktop->gui = furi_record_open("gui");
    desktop->scene_thread = furi_thread_alloc();
    desktop->view_dispatcher = view_dispatcher_alloc();
    desktop->scene_manager = scene_manager_alloc(&desktop_scene_handlers, desktop);
    desktop->animation = desktop_animation_alloc();

    view_dispatcher_enable_queue(desktop->view_dispatcher);
    view_dispatcher_attach_to_gui(
        desktop->view_dispatcher, desktop->gui, ViewDispatcherTypeDesktop);

    view_dispatcher_set_event_callback_context(desktop->view_dispatcher, desktop);
    view_dispatcher_set_custom_event_callback(
        desktop->view_dispatcher, desktop_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        desktop->view_dispatcher, desktop_back_event_callback);

    desktop->main_view = desktop_main_alloc();
    desktop->lock_menu = desktop_lock_menu_alloc();
    desktop->locked_view = desktop_locked_alloc();
    desktop->debug_view = desktop_debug_alloc();
    desktop->first_start_view = desktop_first_start_alloc();
    desktop->hw_mismatch_popup = popup_alloc();
    desktop->code_input = code_input_alloc();

    view_dispatcher_add_view(
        desktop->view_dispatcher, DesktopViewMain, desktop_main_get_view(desktop->main_view));
    view_dispatcher_add_view(
        desktop->view_dispatcher,
        DesktopViewLockMenu,
        desktop_lock_menu_get_view(desktop->lock_menu));
    view_dispatcher_add_view(
        desktop->view_dispatcher, DesktopViewDebug, desktop_debug_get_view(desktop->debug_view));
    view_dispatcher_add_view(
        desktop->view_dispatcher,
        DesktopViewLocked,
        desktop_locked_get_view(desktop->locked_view));
    view_dispatcher_add_view(
        desktop->view_dispatcher,
        DesktopViewFirstStart,
        desktop_first_start_get_view(desktop->first_start_view));
    view_dispatcher_add_view(
        desktop->view_dispatcher,
        DesktopViewHwMismatch,
        popup_get_view(desktop->hw_mismatch_popup));
    view_dispatcher_add_view(
        desktop->view_dispatcher, DesktopViewPinSetup, code_input_get_view(desktop->code_input));
    // Lock icon
    desktop->lock_viewport = view_port_alloc();
    view_port_set_width(desktop->lock_viewport, icon_get_width(&I_Lock_8x8));
    view_port_draw_callback_set(desktop->lock_viewport, desktop_lock_icon_callback, desktop);
    view_port_enabled_set(desktop->lock_viewport, false);
    gui_add_view_port(desktop->gui, desktop->lock_viewport, GuiLayerStatusBarLeft);

    return desktop;
}

void desktop_free(Desktop* desktop) {
    furi_assert(desktop);

    desktop_animation_free(desktop->animation);
    view_dispatcher_remove_view(desktop->view_dispatcher, DesktopViewMain);
    view_dispatcher_remove_view(desktop->view_dispatcher, DesktopViewLockMenu);
    view_dispatcher_remove_view(desktop->view_dispatcher, DesktopViewLocked);
    view_dispatcher_remove_view(desktop->view_dispatcher, DesktopViewDebug);
    view_dispatcher_remove_view(desktop->view_dispatcher, DesktopViewFirstStart);
    view_dispatcher_remove_view(desktop->view_dispatcher, DesktopViewHwMismatch);
    view_dispatcher_remove_view(desktop->view_dispatcher, DesktopViewPinSetup);

    view_dispatcher_free(desktop->view_dispatcher);
    scene_manager_free(desktop->scene_manager);

    desktop_main_free(desktop->main_view);
    desktop_lock_menu_free(desktop->lock_menu);
    desktop_locked_free(desktop->locked_view);
    desktop_debug_free(desktop->debug_view);
    desktop_first_start_free(desktop->first_start_view);
    popup_free(desktop->hw_mismatch_popup);
    code_input_free(desktop->code_input);

    furi_record_close("gui");
    desktop->gui = NULL;

    furi_thread_free(desktop->scene_thread);

    furi_record_close("menu");

    free(desktop);
}

static bool desktop_is_first_start() {
    Storage* storage = furi_record_open("storage");
    bool exists = storage_common_stat(storage, "/int/first_start", NULL) == FSE_OK;
    furi_record_close("storage");

    return exists;
}

static void desktop_dolphin_state_changed_callback(const void* message, void* context) {
    Desktop* desktop = context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, DesktopMainEventUpdateAnimation);
}

static void desktop_storage_state_changed_callback(const void* message, void* context) {
    Desktop* desktop = context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, DesktopMainEventUpdateAnimation);
}

int32_t desktop_srv(void* p) {
    Desktop* desktop = desktop_alloc();

    Dolphin* dolphin = furi_record_open("dolphin");
    FuriPubSub* dolphin_pubsub = dolphin_get_pubsub(dolphin);
    FuriPubSubSubscription* dolphin_subscription =
        furi_pubsub_subscribe(dolphin_pubsub, desktop_dolphin_state_changed_callback, desktop);

    Storage* storage = furi_record_open("storage");
    FuriPubSub* storage_pubsub = storage_get_pubsub(storage);
    FuriPubSubSubscription* storage_subscription =
        furi_pubsub_subscribe(storage_pubsub, desktop_storage_state_changed_callback, desktop);

    bool loaded = LOAD_DESKTOP_SETTINGS(&desktop->settings);
    if(!loaded) {
        furi_hal_rtc_reset_flag(FuriHalRtcFlagLock);
        memset(&desktop->settings, 0, sizeof(desktop->settings));
        SAVE_DESKTOP_SETTINGS(&desktop->settings);
    }

    scene_manager_next_scene(desktop->scene_manager, DesktopSceneMain);

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagLock)) {
        furi_hal_usb_disable();
        scene_manager_set_scene_state(
            desktop->scene_manager, DesktopSceneLocked, DesktopLockedWithPin);
        scene_manager_next_scene(desktop->scene_manager, DesktopSceneLocked);
    }

    if(desktop_is_first_start()) {
        scene_manager_next_scene(desktop->scene_manager, DesktopSceneFirstStart);
    }

    if(!furi_hal_version_do_i_belong_here()) {
        scene_manager_next_scene(desktop->scene_manager, DesktopSceneHwMismatch);
    }

    if(furi_hal_rtc_get_fault_data()) {
        scene_manager_next_scene(desktop->scene_manager, DesktopSceneFault);
    }

    view_dispatcher_run(desktop->view_dispatcher);
    furi_pubsub_unsubscribe(dolphin_pubsub, dolphin_subscription);
    furi_pubsub_unsubscribe(storage_pubsub, storage_subscription);
    desktop_free(desktop);

    return 0;
}
