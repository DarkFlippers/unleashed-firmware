// The editor: opening a file, its input loop and the Save and Cut actions.

#include "app.h"
#include <furi_hal_power.h>

// Save A..B as RAW, or with dc/d the decoded entry d as a key file.
void prompt_and_save(
    Gui* gui,
    ViewPort* vp,
    FuriMessageQueue* queue,
    Storage* st,
    App* a,
    DecodeCtx* dc,
    const Decoded* d) {
    char namebuf[64];
    if(d) {
        char base[48];
        decoded_base_name(dc, d, base, sizeof(base));
        propose_name(st, base, "_", true, namebuf, sizeof(namebuf));
    } else
        propose_edit_name(st, a, namebuf, sizeof(namebuf));

    FuriString* path = furi_string_alloc();
    bool write = false;
    snprintf(a->status, sizeof(a->status), "Cancelled");
    while(prompt_filename(gui, namebuf, sizeof(namebuf)) && namebuf[0] != '\0') {
        furi_string_printf(path, "%s/%s.sub", SUBGHZ_DIR, namebuf);
        if(!storage_file_exists(st, furi_string_get_cstr(path)))
            write = true;
        else if(a->src_in_dir && strcasecmp(namebuf, a->basename) == 0)
            snprintf(a->status, sizeof(a->status), "Can't overwrite open file");
        else if(!confirm(gui, "Name exists,\noverwrite?"))
            continue;
        else
            write = true;

        break;
    }

    if(write) {
        a->busy = "Saving...";
        view_port_update(vp);

        const char* p = furi_string_get_cstr(path);
        if(d ? write_decoded(st, a, dc, d, p) : write_selection(st, a, p))
            snprintf(a->status, sizeof(a->status), "Saved\n%s.sub", namebuf);

        a->busy = NULL;
    }

    furi_string_free(path);

    a->status_until = furi_get_tick() + 2000;
    furi_message_queue_reset(queue);
    view_port_update(vp);
}

// The menu's Cut / Copy / Paste item.
static void do_edit_action(ViewPort* vp, FuriMessageQueue* queue, DialogsApp* dialogs, App* a) {
    furi_mutex_acquire(a->mutex, FuriWaitForever);
    a->status_until = furi_get_tick() + 1500;
    bool paste = a->edit_op == EditPaste;

    if(paste && !a->clip_count) {
        snprintf(a->status, sizeof(a->status), "Nothing copied yet");
        furi_mutex_release(a->mutex);
        return;
    }

    if(!paste && iabs32(a->marker_b - a->marker_a) < 1) {
        snprintf(a->status, sizeof(a->status), "Range too small");
        furi_mutex_release(a->mutex);
        return;
    }

    if(a->edit_op == EditCopy) {
        bool ok = clip_store(a);
        snprintf(
            a->status,
            sizeof(a->status),
            ok ? "Copied %u samples" : "Copy failed",
            (unsigned)a->clip_count);
        furi_mutex_release(a->mutex);
        return;
    }

    if(!paste && a->sd.file_count - a->sd.count + cut_run(a, false) < 2) {
        furi_mutex_release(a->mutex);

        show_message(dialogs, "Cannot cut", "That would leave the\nsignal empty.", 30);
        furi_message_queue_reset(queue);
        return;
    }

    // Like a copy, a cut puts A..B on the clipboard
    if(!paste) clip_store(a);

    // The wait screen draws nothing of the signal, so the file can be rewritten unlocked.
    a->busy = paste ? "Pasting..." : "Cutting...";
    furi_mutex_release(a->mutex);
    view_port_update(vp);

    bool ok = paste ? work_paste(a) : work_cut(a);

    furi_mutex_acquire(a->mutex, FuriWaitForever);
    a->busy = NULL;
    if(ok)
        snprintf(a->status, sizeof(a->status), paste ? "Pasted" : "Cut");
    else
        snprintf(
            a->status, sizeof(a->status), "%s failed\nSD card error", paste ? "Paste" : "Cut");

    a->status_until = furi_get_tick() + 1200;
    furi_mutex_release(a->mutex);
}

/* Undo one step, or with more of them pick from the history - newest first -
 * the step to go back before. */
static void do_undo_action(Gui* gui, FuriMessageQueue* queue, App* a) {
    size_t k = a->undo_n - 1;
    if(!a->undo_n) {
        snprintf(a->status, sizeof(a->status), "Nothing to undo");
        a->status_until = furi_get_tick() + 1500;
        return;
    }

    if(a->undo_n > 1) {
        PromptCtx pc = {0};
        Submenu* sm = submenu_alloc();
        submenu_set_header(sm, "Undo back to before");
        char label[32];
        for(size_t i = a->undo_n; i-- > 0;) {
            undo_label(a, i, label, sizeof(label));
            submenu_add_item(sm, label, i, pick_index_cb, &pc);
        }

        bool ok = prompt_run(gui, &pc, submenu_get_view(sm));
        submenu_free(sm);
        furi_message_queue_reset(queue);
        if(!ok) return;

        k = pc.index;
    }

    furi_mutex_acquire(a->mutex, FuriWaitForever);
    bool ok = work_undo(a, k);
    snprintf(a->status, sizeof(a->status), ok ? "Undone" : "Undo failed\nSD card error");
    a->status_until = furi_get_tick() + 1500;
    furi_mutex_release(a->mutex);
}

// Move the active marker, sliding the RAM window when it is pushed past an edge.
static void move_marker(App* a, int32_t step) {
    int32_t* m = a->active ? &a->marker_b : &a->marker_a;
    *m += step;
    int32_t lo = a->sd.win_t0, hi = lo + a->sd.win_us;
    if((*m < lo || *m > hi) && work_slide(a, *m > hi ? 1 : -1))
        clamp_marker(a, !a->active); // the other one may have left the window

    clamp_marker(a, a->active);
    ensure_visible(a, *m);
}

void run_editor_session(
    Gui* gui,
    ViewPort* vp,
    FuriMessageQueue* queue,
    Storage* storage,
    DialogsApp* dialogs,
    App* app,
    bool auto_select) {
    app->busy = NULL;
    app->opened_tick = furi_get_tick();
    if(auto_select) {
        auto_detect(app);
    } else {
        app->marker_a = 0;
        app->marker_b = app->sd.win_us;
        app->view_start = 0;
        app->view_end = app->sd.win_us > 0 ? app->sd.win_us : 1;
        clamp_view(app);
    }
    recompute_activity(app);
    furi_message_queue_reset(queue);
    view_port_update(vp);

    bool running = true;
    InputEvent e;
    while(running) {
        if(furi_message_queue_get(queue, &e, 100) == FuriStatusOk) {
            furi_mutex_acquire(app->mutex, FuriWaitForever);
            bool changed = false;
            bool want_save = false;
            bool want_decode = false;
            bool want_cut = false;
            bool want_undo = false;

            if(app->mode == EditModeMenu) {
                if(e.type == InputTypeShort) {
                    if(e.key == InputKeyDown) {
                        app->menu_sel = (app->menu_sel + 1) % MENU_ITEMS;
                    } else if(e.key == InputKeyUp) {
                        app->menu_sel = (app->menu_sel + MENU_ITEMS - 1) % MENU_ITEMS;
                    } else if(
                        app->menu_sel == MENU_SEL_CUT &&
                        (e.key == InputKeyLeft || e.key == InputKeyRight)) {
                        app->edit_op =
                            (app->edit_op + (e.key == InputKeyRight ? 1 : EditOps - 1)) % EditOps;
                    } else if(e.key == InputKeyOk) {
                        app->mode = EditModeEdit;
                        if(app->menu_sel == MENU_SEL_SAVE) {
                            want_save = true;
                        } else if(app->menu_sel == MENU_SEL_DECODE) {
                            want_decode = true;
                        } else if(app->menu_sel == MENU_SEL_CUT) {
                            want_cut = true;
                        } else if(app->menu_sel == MENU_SEL_UNDO) {
                            want_undo = true;
                        }
                    } else if(e.key == InputKeyBack) {
                        app->mode = EditModeEdit;
                    }
                }
            } else if(e.type == InputTypePress || e.type == InputTypeRepeat) {
                int32_t span = app->view_end - app->view_start;
                int32_t step = span / SCREEN_W_PX;
                if(step < 1) step = 1;

                if(e.type == InputTypeRepeat) {
                    int32_t fast = span / 24;
                    if(fast > step) step = fast;
                }

                switch(e.key) {
                case InputKeyLeft:
                    move_marker(app, -step);
                    changed = true;
                    break;
                case InputKeyRight:
                    move_marker(app, step);
                    changed = true;
                    break;
                case InputKeyUp:
                    zoom(app, true);
                    changed = true;
                    break;
                case InputKeyDown:
                    zoom(app, false);
                    changed = true;
                    break;
                default:
                    break;
                }
            } else if(e.type == InputTypeShort) {
                if(e.key == InputKeyOk) {
                    app->active ^= 1;
                    ensure_visible(app, app->active ? app->marker_b : app->marker_a);
                    changed = true;
                } else if(e.key == InputKeyBack) {
                    running = false;
                }
            } else if(e.type == InputTypeLong) {
                if(e.key == InputKeyOk) {
                    app->mode = EditModeMenu;
                    app->menu_sel = 0;
                    selection_info(app);
                } else if(e.key == InputKeyBack) {
                    running = false;
                }
            }

            if(changed) recompute_activity(app);

            furi_mutex_release(app->mutex);

            if(want_save) {
                prompt_and_save(gui, vp, queue, storage, app, NULL, NULL);
            } else if(want_decode) {
                do_decode_action(gui, vp, queue, storage, app);
            } else if(want_cut) {
                do_edit_action(vp, queue, dialogs, app);
            } else if(want_undo) {
                do_undo_action(gui, queue, app);
            }

            view_port_update(vp);
        } else {
            view_port_update(vp);
        }
    }
}

/* Once the working file is written: take the RAM window, and when RAM holds
 * only a small part of the signal offer a reboot for a bigger one. False when
 * the editor can't open. */
bool open_work(DialogsApp* dialogs, App* app) {
    if(!work_open(app)) {
        show_message(
            dialogs,
            app->sd.out_of_memory ? "Out of memory" : "SD card error",
            app->sd.out_of_memory ?
                "Not enough free RAM.\nReboot Flipper, then\nopen the app first." :
                "Can't read the\nworking copy.",
            34);
        return false;
    }

    if(!work_windowed(&app->sd) || app->sd.cap >= WINDOW_WARN_SAMPLES) return true;

    char msg[64];
    snprintf(
        msg,
        sizeof(msg),
        "RAM fits %u samples\nat once. Reboot for\na bigger window.",
        (unsigned)app->sd.cap);
    DialogMessage* m = dialog_message_alloc();
    dialog_message_set_header(m, "Low RAM", 64, 2, AlignCenter, AlignTop);
    dialog_message_set_text(m, msg, 64, 32, AlignCenter, AlignCenter);
    dialog_message_set_buttons(m, "Reboot", NULL, "Continue");
    DialogMessageButton b = dialog_message_show(dialogs, m);
    dialog_message_free(m);

    if(b == DialogMessageButtonLeft) {
        work_cleanup(app->st);
        furi_hal_power_reset();
    }

    return b == DialogMessageButtonRight;
}

// Edit `file`, or a file picked in the browser when NULL.
void run_editor(Storage* storage, DialogsApp* dialogs, const char* file) {
    App* app = safe_malloc(sizeof(App));
    if(!app) {
        show_message(
            dialogs, "Out of memory", "Not enough RAM to\nopen the editor.\nReboot Flipper.", 32);
        return;
    }

    memset(app, 0, sizeof(App));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->st = storage;

    FuriString* path = furi_string_alloc_set(file ? file : SUBGHZ_DIR);
    if(!file) {
        DialogsFileBrowserOptions br;
        dialog_file_browser_set_basic_options(&br, ".sub", &I_sub1_10px);
        br.base_path = SUBGHZ_DIR;
        if(!dialog_file_browser_show(dialogs, path, path, &br)) goto cleanup;
    }

    {
        const char* full = furi_string_get_cstr(path);
        const char* slash = strrchr(full, '/');
        const char* nm = slash ? slash + 1 : full;
        strncpy(app->basename, nm, sizeof(app->basename) - 1);
        char* dot = strrchr(app->basename, '.');
        if(dot) *dot = '\0';

        size_t dl = strlen(SUBGHZ_DIR);
        app->src_in_dir = slash && (size_t)(slash - full) == dl &&
                          strncmp(full, SUBGHZ_DIR, dl) == 0 && strlen(nm) < sizeof(app->basename);
    }

    Gui* gui = furi_record_open(RECORD_GUI);

    // Loading screen first: probing the buttons takes a moment and the menu would show meanwhile
    ViewPort* vp = view_port_alloc();
    FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    view_port_draw_callback_set(vp, draw_cb, app);
    view_port_input_callback_set(vp, input_cb, queue);
    app->busy = busy_loading;
    gui_add_view_port(gui, vp, GuiLayerFullscreen);
    view_port_update(vp);

    if(!btn_select_for_file(gui, storage, furi_string_get_cstr(path))) {
        gui_remove_view_port(gui, vp);
        view_port_free(vp);
        furi_message_queue_free(queue);
        furi_record_close(RECORD_GUI);
        goto cleanup;
    }
    furi_message_queue_reset(queue);

    app->sd.frequency = 433920000;
    strncpy(app->sd.preset, "FuriHalSubGhzPresetOok650Async", sizeof(app->sd.preset) - 1);
    WorkWriter* w = work_begin(storage);
    bool loaded = w &&
                  work_add_sub(storage, furi_string_get_cstr(path), w, &app->sd, MergeJoinNone);
    loaded = w && work_end(w, &app->sd) && loaded;

    if(!loaded)
        show_message(dialogs, APP_NAME, "Unsupported protocol,\nempty file or SD error", 32);

    if(loaded && open_work(dialogs, app)) {
        furi_message_queue_reset(queue);
        run_editor_session(gui, vp, queue, storage, dialogs, app, true);
    }

    gui_remove_view_port(gui, vp);
    view_port_free(vp);
    furi_message_queue_free(queue);
    furi_record_close(RECORD_GUI);

cleanup:
    furi_string_free(path);

    if(app->sd.data) free(app->sd.data);

    free(app->undo);
    work_cleanup(storage);
    furi_mutex_free(app->mutex);
    free(app);
}
