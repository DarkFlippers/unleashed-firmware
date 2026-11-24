// Modal screens: the custom button picker, prompts and messages.

#include "app.h"

/* ---- Custom button picker UI ------------------------------------------- */

#ifndef FW_ORIGIN_Official

static void btn_pick_fit(Canvas* c, const char* s, int maxw, char* out, size_t outlen) {
    strncpy(out, s, outlen - 1);
    out[outlen - 1] = '\0';

    if(canvas_string_width(c, out) <= (size_t)maxw) return;

    size_t n = strlen(out);
    while(n > 1) {
        out[--n] = '\0';

        char tmp[BTN_LABEL_LEN + 8];
        snprintf(tmp, sizeof(tmp), "%s..", out);
        if(canvas_string_width(c, tmp) <= (size_t)maxw) {
            strncpy(out, tmp, outlen - 1);
            out[outlen - 1] = '\0';
            return;
        }
    }
}

static void btn_pick_draw_cb(Canvas* c, void* ctx) {
    const BtnPick* bp = ctx;

    canvas_clear(c);
    canvas_set_font(c, FontSecondary);
    canvas_draw_str(c, 0, 7, bp->protocol);
    canvas_draw_str_aligned(c, 127, 7, AlignRight, AlignBottom, "Back: cancel");
    canvas_draw_line(c, 0, 9, 127, 9);

    const int cx = 21, cy = 38;

    if(bp->label[SUBGHZ_CUSTOM_BTN_OK][0]) canvas_draw_circle(c, cx, cy, 5);

    if(bp->label[SUBGHZ_CUSTOM_BTN_UP][0])
        canvas_draw_triangle(c, cx, cy - 8, 9, 6, CanvasDirectionBottomToTop);

    if(bp->label[SUBGHZ_CUSTOM_BTN_DOWN][0])
        canvas_draw_triangle(c, cx, cy + 8, 9, 6, CanvasDirectionTopToBottom);

    if(bp->label[SUBGHZ_CUSTOM_BTN_LEFT][0])
        canvas_draw_triangle(c, cx - 8, cy, 9, 6, CanvasDirectionRightToLeft);

    if(bp->label[SUBGHZ_CUSTOM_BTN_RIGHT][0])
        canvas_draw_triangle(c, cx + 8, cy, 9, 6, CanvasDirectionLeftToRight);

    static const uint8_t order[BTN_SLOTS] = {
        SUBGHZ_CUSTOM_BTN_OK,
        SUBGHZ_CUSTOM_BTN_UP,
        SUBGHZ_CUSTOM_BTN_DOWN,
        SUBGHZ_CUSTOM_BTN_LEFT,
        SUBGHZ_CUSTOM_BTN_RIGHT,
    };

    for(int row = 0; row < BTN_SLOTS; row++) {
        uint8_t slot = order[row];
        int y = 19 + row * 9;
        int gx = 49, gy = y - 2;

        switch(slot) {
        case SUBGHZ_CUSTOM_BTN_UP:
            canvas_draw_triangle(c, gx, gy + 2, 5, 4, CanvasDirectionBottomToTop);
            break;
        case SUBGHZ_CUSTOM_BTN_DOWN:
            canvas_draw_triangle(c, gx, gy - 2, 5, 4, CanvasDirectionTopToBottom);
            break;
        case SUBGHZ_CUSTOM_BTN_LEFT:
            canvas_draw_triangle(c, gx + 2, gy, 5, 4, CanvasDirectionRightToLeft);
            break;
        case SUBGHZ_CUSTOM_BTN_RIGHT:
            canvas_draw_triangle(c, gx - 2, gy, 5, 4, CanvasDirectionLeftToRight);
            break;
        default:
            canvas_draw_disc(c, gx, gy, 2);
            break;
        }

        char buf[BTN_LABEL_LEN + 4];
        btn_pick_fit(c, bp->label[slot][0] ? bp->label[slot] : "-", 127 - 56, buf, sizeof(buf));
        canvas_draw_str(c, 56, y, buf);
    }
}

static int btn_pick_dialog(Gui* gui, const BtnPick* bp) {
    ViewPort* vp = view_port_alloc();
    FuriMessageQueue* q = furi_message_queue_alloc(8, sizeof(InputEvent));

    view_port_draw_callback_set(vp, btn_pick_draw_cb, (void*)bp);
    view_port_input_callback_set(vp, input_cb, q);
    gui_add_view_port(gui, vp, GuiLayerFullscreen);
    view_port_update(vp);

    int sel = -1;
    InputEvent e;

    while(true) {
        if(furi_message_queue_get(q, &e, FuriWaitForever) != FuriStatusOk) continue;

        if(e.type != InputTypeShort && e.type != InputTypeLong) continue;

        if(e.key == InputKeyBack) break;

        uint8_t slot;
        if(e.key == InputKeyOk)
            slot = SUBGHZ_CUSTOM_BTN_OK;
        else if(e.key == InputKeyUp)
            slot = SUBGHZ_CUSTOM_BTN_UP;
        else if(e.key == InputKeyDown)
            slot = SUBGHZ_CUSTOM_BTN_DOWN;
        else if(e.key == InputKeyLeft)
            slot = SUBGHZ_CUSTOM_BTN_LEFT;
        else if(e.key == InputKeyRight)
            slot = SUBGHZ_CUSTOM_BTN_RIGHT;
        else
            continue;

        if(!bp->label[slot][0]) continue;

        sel = (int)slot;
        break;
    }

    gui_remove_view_port(gui, vp);
    view_port_free(vp);
    furi_message_queue_free(q);
    return sel;
}

static bool read_protocol(Storage* storage, const char* path, char* out, size_t outlen) {
    File* f = storage_file_alloc(storage);
    if(!storage_file_open(f, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(f);
        return false;
    }

    LineReader lr = {.file = f, .len = 0, .pos = 0, .eof = false};
    FuriString* line = furi_string_alloc();
    uint32_t freq = 0;
    char preset[48] = {0};

    out[0] = '\0';
    while(lr_read_line(&lr, line)) {
        const char* s = furi_string_get_cstr(line);
        if(strncmp(s, "RAW_Data:", 9) == 0) break;

        parse_sub_meta(s, &freq, preset, sizeof(preset), out, outlen);
    }

    furi_string_free(line);
    storage_file_close(f);
    storage_file_free(f);
    return true;
}

#endif // FW_ORIGIN_Official

bool btn_select_for_file(Gui* gui, Storage* storage, const char* path) {
    g_custom_btn_id = SUBGHZ_CUSTOM_BTN_OK;

#ifdef FW_ORIGIN_Official
    UNUSED(gui);
    UNUSED(storage);
    UNUSED(path);
    return true;
#else
    subghz_custom_btns_reset();

    char protocol[32] = {0};
    if(!read_protocol(storage, path, protocol, sizeof(protocol))) return true;

    if(!protocol[0] || strcmp(protocol, "RAW") == 0) return true;

    BtnPick bp;
    btn_probe(storage, path, protocol, &bp);
    if(!bp.available) return true;

    int sel = btn_pick_dialog(gui, &bp);
    if(sel < 0) return false;

    g_custom_btn_id = (uint8_t)sel;
    return true;
#endif
}

static void prompt_result_cb(void* ctx) {
    PromptCtx* p = ctx;
    p->confirmed = true;
    view_dispatcher_stop(p->vd);
}

void pick_index_cb(void* ctx, uint32_t index) {
    PromptCtx* p = ctx;
    p->index = index;
    prompt_result_cb(p);
}

// Widget buttons: Left declines, any other confirms.
void button_ok_cb(GuiButtonType button, InputType type, void* ctx) {
    PromptCtx* p = ctx;
    if(type != InputTypeShort) return;

    p->confirmed = button != GuiButtonTypeLeft;
    p->index = button;
    view_dispatcher_stop(p->vd);
}

static uint32_t prompt_back_cb(void* ctx) {
    UNUSED(ctx);
    return VIEW_NONE;
}

// Run one view modally until it confirms (true) or Back leaves it (false).
bool prompt_run(Gui* gui, PromptCtx* pc, View* view) {
    pc->vd = view_dispatcher_alloc();
    view_set_previous_callback(view, prompt_back_cb);
    view_dispatcher_attach_to_gui(pc->vd, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_add_view(pc->vd, 0, view);
    view_dispatcher_switch_to_view(pc->vd, 0);
    view_dispatcher_run(pc->vd);
    view_dispatcher_remove_view(pc->vd, 0);
    view_dispatcher_free(pc->vd);
    return pc->confirmed;
}

bool prompt_filename(Gui* gui, char* namebuf, size_t buflen) {
    PromptCtx pc = {0};
    TextInput* ti = text_input_alloc();
    text_input_set_header_text(ti, "Name the file");
    text_input_set_result_callback(ti, prompt_result_cb, &pc, namebuf, buflen, false);
    bool ok = prompt_run(gui, &pc, text_input_get_view(ti));
    text_input_free(ti);
    return ok;
}

bool confirm(Gui* gui, const char* question) {
    PromptCtx pc = {0};
    Widget* w = widget_alloc();
    widget_add_string_multiline_element(
        w, 64, 24, AlignCenter, AlignCenter, FontPrimary, question);
    widget_add_button_element(w, GuiButtonTypeLeft, "No", button_ok_cb, &pc);
    widget_add_button_element(w, GuiButtonTypeRight, "Yes", button_ok_cb, &pc);
    bool ok = prompt_run(gui, &pc, widget_get_view(w));
    widget_free(w);
    return ok;
}

// Message with a header and an OK button; text_y centres the text.
void show_message(DialogsApp* dialogs, const char* header, const char* text, uint8_t text_y) {
    DialogMessage* m = dialog_message_alloc();
    dialog_message_set_header(m, header, 64, 2, AlignCenter, AlignTop);
    dialog_message_set_text(m, text, 64, text_y, AlignCenter, AlignCenter);
    dialog_message_set_buttons(m, NULL, NULL, "OK");
    dialog_message_show(dialogs, m);
    dialog_message_free(m);
}
