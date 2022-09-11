#include "subbrute_scene_entrypoint.h"
#include "../subbrute_utils.h"

string_t subbrute_menu_items[10];

void subbrute_scene_entrypoint_menu_callback(SubBruteState* context, uint32_t index) {
    string_set_str(context->preset, "FuriHalSubGhzPresetOok650Async");
    string_set_str(context->protocol, "RAW");
    context->repeat = 5;
    context->te = 0;
    context->attack = index;
    switch(index) {
    case SubBruteAttackLoadFile:
        context->current_scene = SceneSelectFile;
        break;
    case SubBruteAttackCAME12bit307:
    case SubBruteAttackCAME12bit433:
    case SubBruteAttackCAME12bit868:
        if (index == SubBruteAttackCAME12bit307) {
            context->frequency = 307800000;
        } else if (index == SubBruteAttackCAME12bit433) {
            context->frequency = 433920000;
        } else if (index == SubBruteAttackCAME12bit868) {
            context->frequency = 868350000;
        }
        context->bit = 12;
        string_set_str(context->protocol, "CAME");
        string_set_str(context->preset, "FuriHalSubGhzPresetOok650Async");
        if(!subbrute_is_frequency_allowed(context)) {
            return;
        }
        context->current_scene = SceneAttack;
        break;
    case SubBruteAttackChamberlain9bit315:
        context->frequency = 315000000;
        context->bit = 9;
        string_set_str(context->protocol, "Cham_Code");
        string_set_str(context->preset, "FuriHalSubGhzPresetOok650Async");

        if(!subbrute_is_frequency_allowed(context)) {
            return;
        }
        context->current_scene = SceneAttack;
        break;
    case SubBruteAttackChamberlain9bit390:
        context->frequency = 390000000;
        context->bit = 9;
        string_set_str(context->protocol, "Cham_Code");
        string_set_str(context->preset, "FuriHalSubGhzPresetOok650Async");

        if(!subbrute_is_frequency_allowed(context)) {
            return;
        }
        context->current_scene = SceneAttack;
        break;
    case SubBruteAttackLinear10bit300:
        context->frequency = 300000000;
        context->bit = 10;
        string_set_str(context->protocol, "Linear");
        string_set_str(context->preset, "FuriHalSubGhzPresetOok650Async");
        if(!subbrute_is_frequency_allowed(context)) {
            return;
        }
        context->current_scene = SceneAttack;
        break;
    case SubBruteAttackLinear10bit310:
        context->frequency = 310000000;
        context->bit = 10;
        string_set_str(context->protocol, "Linear");
        string_set_str(context->preset, "FuriHalSubGhzPresetOok650Async");
        if(!subbrute_is_frequency_allowed(context)) {
            return;
        }
        context->current_scene = SceneAttack;
        break;
    case SubBruteAttackNICE12bit433:
        context->frequency = 433920000;
        context->bit = 12;
        string_set_str(context->protocol, "Nice FLO");
        string_set_str(context->preset, "FuriHalSubGhzPresetOok650Async");
        if(!subbrute_is_frequency_allowed(context)) {
            return;
        }
        context->current_scene = SceneAttack;
        break;
    case SubBruteAttackNICE12bit868:
        context->frequency = 868350000;
        context->bit = 12;
        string_set_str(context->protocol, "Nice FLO");
        string_set_str(context->preset, "FuriHalSubGhzPresetOok650Async");
        if(!subbrute_is_frequency_allowed(context)) {
            return;
        }
        context->current_scene = SceneAttack;
        break;
    default:
        break;
    }
}

void subbrute_scene_entrypoint_on_enter(SubBruteState* context) {
    // Clear the previous payload
    context->menu_index = 0;
    for(uint32_t i = 0; i < 10; i++) {
        string_init(subbrute_menu_items[i]);
    }

    string_set(subbrute_menu_items[0], "BF existing dump");
    string_set(subbrute_menu_items[1], "CAME 12bit 307mhz");
    string_set(subbrute_menu_items[2], "CAME 12bit 433mhz");
    string_set(subbrute_menu_items[3], "CAME 12bit 868mhz");
    string_set(subbrute_menu_items[4], "Chamberlain 9bit 315mhz");
    string_set(subbrute_menu_items[5], "Chamberlain 9bit 390mhz");
    string_set(subbrute_menu_items[6], "Linear 10bit 300mhz");
    string_set(subbrute_menu_items[7], "Linear 10bit 310mhz");
    string_set(subbrute_menu_items[8], "NICE 12bit 433mhz");
    string_set(subbrute_menu_items[9], "NICE 12bit 868mhz");
}

void subbrute_scene_entrypoint_on_exit(SubBruteState* context) {
    UNUSED(context);
    for(uint32_t i = 0; i < 10; i++) {
        string_clear(subbrute_menu_items[i]);
    }
}

void subbrute_scene_entrypoint_on_tick(SubBruteState* context) {
    UNUSED(context);
}

void subbrute_scene_entrypoint_on_event(SubBruteEvent event, SubBruteState* context) {
    if(event.evt_type == EventTypeKey) {
        if(event.input_type == InputTypeShort) {
            switch(event.key) {
            case InputKeyDown:
                if(context->menu_index < SubBruteAttackNICE12bit868) {
                    context->menu_index++;
                }
                break;
            case InputKeyUp:
                if(context->menu_index > SubBruteAttackLoadFile) {
                    context->menu_index--;
                }
                break;
            case InputKeyLeft:
            case InputKeyRight:
                break;
            case InputKeyOk:
                subbrute_scene_entrypoint_menu_callback(context, context->menu_index);
                break;
            case InputKeyBack:
                context->is_running = false;
                break;
            }
        }
    }
}

void subbrute_scene_entrypoint_on_draw(Canvas* canvas, SubBruteState* context) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    // Title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 6, AlignCenter, AlignTop, "Sub-GHz Bruteforcer");

    if(context->menu_index > SubBruteAttackLoadFile) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(
            canvas,
            64,
            24,
            AlignCenter,
            AlignTop,
            string_get_cstr(subbrute_menu_items[context->menu_index - 1]));
    }

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(
        canvas,
        64,
        36,
        AlignCenter,
        AlignTop,
        string_get_cstr(subbrute_menu_items[context->menu_index]));

    if(context->menu_index < SubBruteAttackNICE12bit868) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(
            canvas,
            64,
            48,
            AlignCenter,
            AlignTop,
            string_get_cstr(subbrute_menu_items[context->menu_index + 1]));
    }
}