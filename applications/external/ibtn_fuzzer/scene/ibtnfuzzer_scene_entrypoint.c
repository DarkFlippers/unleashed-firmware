#include "ibtnfuzzer_scene_entrypoint.h"

FuriString* main_menu_items[3];
FuriString* main_menu_proto_items[3];

void ibtnfuzzer_scene_entrypoint_menu_callback(
    iBtnFuzzerState* context,
    uint32_t index,
    uint32_t proto_index) {
    switch(index) {
    case iBtnFuzzerAttackDefaultValues:
        context->attack = iBtnFuzzerAttackDefaultValues;
        context->current_scene = SceneAttack;
        furi_string_set(context->attack_name, "Default Values");
        break;
    case iBtnFuzzerAttackLoadFile:
        context->attack = iBtnFuzzerAttackLoadFile;
        context->current_scene = SceneSelectFile;
        furi_string_set(context->attack_name, "Load File");
        break;
    case iBtnFuzzerAttackLoadFileCustomUids:
        context->attack = iBtnFuzzerAttackLoadFileCustomUids;
        context->current_scene = SceneLoadCustomUids;
        furi_string_set(context->attack_name, "Load Custom UIDs");
        break;
    default:
        break;
    }

    switch(proto_index) {
    case DS1990:
        context->proto = DS1990;
        furi_string_set(context->proto_name, "DS1990");
        break;
    case Metakom:
        context->proto = Metakom;
        furi_string_set(context->proto_name, "Metakom");
        break;
    case Cyfral:
        context->proto = Cyfral;
        furi_string_set(context->proto_name, "Cyfral");
        break;
    default:
        break;
    }
}

void ibtnfuzzer_scene_entrypoint_on_enter(iBtnFuzzerState* context) {
    // Clear the previous payload
    context->payload[0] = 0x00;
    context->payload[1] = 0x00;
    context->payload[2] = 0x00;
    context->payload[3] = 0x00;
    context->payload[4] = 0x00;
    context->payload[5] = 0x00;
    context->payload[6] = 0x00;
    context->payload[7] = 0x00;

    context->menu_index = 0;
    /*for(uint32_t i = 0; i < 4; i++) {
        menu_items[i] = furi_string_alloc();
    }*/

    main_menu_items[0] = furi_string_alloc_set("Default Values");
    main_menu_items[1] = furi_string_alloc_set("Load File");
    main_menu_items[2] = furi_string_alloc_set("Load UIDs from file");

    context->menu_proto_index = 0;
    /*for(uint32_t i = 0; i < 4; i++) {
        menu_proto_items[i] = furi_string_alloc();
    }*/

    main_menu_proto_items[0] = furi_string_alloc_set("DS1990");
    main_menu_proto_items[1] = furi_string_alloc_set("Metakom");
    main_menu_proto_items[2] = furi_string_alloc_set("Cyfral");
}

void ibtnfuzzer_scene_entrypoint_on_exit(iBtnFuzzerState* context) {
    context->enter_rerun = false;

    for(uint32_t i = 0; i < 3; i++) {
        furi_string_free(main_menu_items[i]);
    }

    for(uint32_t i = 0; i < 3; i++) {
        furi_string_free(main_menu_proto_items[i]);
    }
}

void ibtnfuzzer_scene_entrypoint_on_tick(iBtnFuzzerState* context) {
    UNUSED(context);
}

void ibtnfuzzer_scene_entrypoint_on_event(iBtnFuzzerEvent event, iBtnFuzzerState* context) {
    if(event.evt_type == EventTypeKey) {
        if(event.input_type == InputTypeShort) {
            switch(event.key) {
            case InputKeyDown:
                if(context->menu_index < iBtnFuzzerAttackLoadFileCustomUids) {
                    context->menu_index++;
                }
                break;
            case InputKeyUp:
                if(context->menu_index > iBtnFuzzerAttackDefaultValues) {
                    context->menu_index--;
                }
                break;
            case InputKeyLeft:
                if(context->menu_proto_index > DS1990) {
                    context->menu_proto_index--;
                } else if(context->menu_proto_index == DS1990) {
                    context->menu_proto_index = Cyfral;
                }
                break;
            case InputKeyRight:
                if(context->menu_proto_index < Cyfral) {
                    context->menu_proto_index++;
                } else if(context->menu_proto_index == Cyfral) {
                    context->menu_proto_index = DS1990;
                }
                break;
            case InputKeyOk:
                ibtnfuzzer_scene_entrypoint_menu_callback(
                    context, context->menu_index, context->menu_proto_index);
                break;
            case InputKeyBack:
                context->is_running = false;
                break;
            default:
                break;
            }
        }
    }
}

void ibtnfuzzer_scene_entrypoint_on_draw(Canvas* canvas, iBtnFuzzerState* context) {
    if(!context->enter_rerun) {
        ibtnfuzzer_scene_entrypoint_on_enter(context);
        context->enter_rerun = true;
    }

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    if(main_menu_items[context->menu_index] != NULL) {
        if(context->menu_index > iBtnFuzzerAttackDefaultValues) {
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str_aligned(
                canvas,
                64,
                24,
                AlignCenter,
                AlignTop,
                furi_string_get_cstr(main_menu_items[context->menu_index - 1]));
        }

        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(
            canvas,
            64,
            36,
            AlignCenter,
            AlignTop,
            furi_string_get_cstr(main_menu_items[context->menu_index]));

        if(context->menu_index < iBtnFuzzerAttackLoadFileCustomUids) {
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str_aligned(
                canvas,
                64,
                48,
                AlignCenter,
                AlignTop,
                furi_string_get_cstr(main_menu_items[context->menu_index + 1]));
        }

        if(context->menu_proto_index > DS1990) {
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str_aligned(
                canvas,
                64,
                -12,
                AlignCenter,
                AlignTop,
                furi_string_get_cstr(main_menu_proto_items[context->menu_proto_index - 1]));
        }

        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 27, 4, AlignCenter, AlignTop, "<");

        canvas_set_font(canvas, FontPrimary);
        if(main_menu_proto_items[context->menu_proto_index] != NULL) {
            canvas_draw_str_aligned(
                canvas,
                64,
                4,
                AlignCenter,
                AlignTop,
                furi_string_get_cstr(main_menu_proto_items[context->menu_proto_index]));
        }
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 101, 4, AlignCenter, AlignTop, ">");

        if(context->menu_proto_index < Cyfral) {
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str_aligned(
                canvas,
                64,
                -12,
                AlignCenter,
                AlignTop,
                furi_string_get_cstr(main_menu_proto_items[context->menu_proto_index + 1]));
        }
    }
}