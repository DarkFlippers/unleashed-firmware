// Merge files: picks several .sub files and joins them into one signal.

#include "app.h"

static void merge_loading_draw_cb(Canvas* c, void* ctx) {
    UNUSED(ctx);
    draw_busy(c, busy_loading, NULL);
}

static DialogMessageButton merge_ask_more(DialogsApp* dialogs, int n, size_t total) {
    char msg[64];
    snprintf(msg, sizeof(msg), "Added %d file(s).\n%u samples.", n, (unsigned)total);
    DialogMessage* m = dialog_message_alloc();
    dialog_message_set_header(m, "Merge .sub files", 64, 2, AlignCenter, AlignTop);
    dialog_message_set_text(m, msg, 64, 32, AlignCenter, AlignCenter);
    dialog_message_set_buttons(m, "Done", NULL, "Add more");
    DialogMessageButton btn = dialog_message_show(dialogs, m);
    dialog_message_free(m);
    return btn;
}

void run_merge(Storage* storage, DialogsApp* dialogs) {
    App* app = safe_malloc(sizeof(App));
    if(!app) {
        show_message(
            dialogs, "Out of memory", "Not enough RAM to\nstart merging.\nReboot Flipper.", 32);
        return;
    }

    memset(app, 0, sizeof(App));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->st = storage;

    // Packed buffer of file paths: <len_byte><path_bytes><len_byte>...<0>.
    // A 0 length byte marks the end (paths are never empty). Grows on demand so
    // the only real limit is available RAM.
    size_t pcap = MERGE_BUF_CHUNK;
    size_t pused = 0;
    uint8_t* pbuf = safe_malloc(pcap);
    if(!pbuf) {
        show_message(
            dialogs, "Out of memory", "Not enough RAM to\nstart merging.\nReboot Flipper.", 32);
        furi_mutex_free(app->mutex);
        free(app);
        return;
    }
    pbuf[0] = 0;

    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* load_vp = view_port_alloc();
    view_port_draw_callback_set(load_vp, merge_loading_draw_cb, NULL);

    int n = 0;
    size_t total = 0;
    bool aborted = false;
    uint32_t freq = 433920000;
    char preset[48];
    strncpy(preset, "FuriHalSubGhzPresetOok650Async", sizeof(preset) - 1);
    preset[sizeof(preset) - 1] = '\0';

    FuriString* path = furi_string_alloc_set(SUBGHZ_DIR);
    DialogsFileBrowserOptions br;
    dialog_file_browser_set_basic_options(&br, ".sub", &I_sub1_10px);
    br.base_path = SUBGHZ_DIR;

    while(true) {
        if(!dialog_file_browser_show(dialogs, path, path, &br)) {
            if(n == 0) break;

            DialogMessageButton b = merge_ask_more(dialogs, n, total);
            if(b == DialogMessageButtonBack) {
                aborted = true;
                break;
            }

            if(b != DialogMessageButtonRight) break;

            continue;
        }

        if(!btn_select_for_file(gui, storage, furi_string_get_cstr(path))) continue;

        gui_add_view_port(gui, load_vp, GuiLayerFullscreen);
        view_port_update(load_vp);
        uint32_t f = freq;
        char pr[48];
        strncpy(pr, preset, sizeof(pr) - 1);
        pr[sizeof(pr) - 1] = '\0';
        size_t cnt = count_sub_samples(storage, furi_string_get_cstr(path), &f, pr, sizeof(pr));
        gui_remove_view_port(gui, load_vp);

        if(cnt == 0) {
            show_message(dialogs, "Unsupported file", "Unsupported protocol or file is empty", 34);
            continue;
        }

        if(n > 0 && f != freq) {
            char msg[128];
            snprintf(
                msg,
                sizeof(msg),
                "Merge: %lu.%02lu MHz\nFile: %lu.%02lu MHz",
                (unsigned long)(freq / 1000000),
                (unsigned long)((freq % 1000000) / 10000),
                (unsigned long)(f / 1000000),
                (unsigned long)((f % 1000000) / 10000));
            show_message(dialogs, "Frequency mismatch", msg, 32);
            continue;
        }

        // Each copy contributes cnt samples. Repetitions reuse the signal's own
        // native trailing silence (no extra samples), while a new file inserts
        // the manual gap, which spans several samples once it exceeds DUR_CLAMP.
        size_t copies = (size_t)g_merge_repeat;
        size_t gap_chunks = duration_samples(g_merge_gap_ms * 1000);
        size_t extra = cnt * copies + (n > 0 ? gap_chunks : 0);
        size_t newtotal = total + extra;

        const char* cstr = furi_string_get_cstr(path);
        size_t plen = strlen(cstr);
        if(plen > MERGE_PATH_LEN - 1) plen = MERGE_PATH_LEN - 1;

        // Append <len_byte><path><btn_byte> to the packed buffer, growing if
        // needed. +3 keeps room for this entry's len byte, the chosen custom
        // button and the final terminator.
        size_t need = pused + 1 + plen + 1 + 1;
        if(need > pcap) {
            size_t newcap = pcap;
            while(newcap < need)
                newcap *= 2;

            uint8_t* np = safe_realloc(pbuf, newcap);
            if(!np) {
                DialogMessage* m = dialog_message_alloc();
                dialog_message_set_header(m, "Merge .sub files", 64, 2, AlignCenter, AlignTop);
                dialog_message_set_text(
                    m,
                    "Out of RAM for more\nfiles. Merging what\nwas added so far.",
                    64,
                    32,
                    AlignCenter,
                    AlignCenter);
                dialog_message_set_buttons(m, NULL, "OK", NULL);
                dialog_message_show(dialogs, m);
                dialog_message_free(m);
                break;
            }

            pbuf = np;
            pcap = newcap;
        }

        pbuf[pused++] = (uint8_t)plen;
        memcpy(pbuf + pused, cstr, plen);
        pused += plen;
        pbuf[pused++] = g_custom_btn_id;
        pbuf[pused] = 0;

        if(n == 0) {
            freq = f;
            strncpy(preset, pr, sizeof(preset) - 1);
            preset[sizeof(preset) - 1] = '\0';
        }

        total = newtotal;
        n++;

        DialogMessageButton b = merge_ask_more(dialogs, n, total);
        if(b == DialogMessageButtonBack) {
            aborted = true;
            break;
        }

        if(b != DialogMessageButtonRight) break;
    }

    furi_string_free(path);

    if(aborted || n < 1 || total < 2) {
        free(pbuf);
        goto cleanup;
    }

    gui_add_view_port(gui, load_vp, GuiLayerFullscreen);
    view_port_update(load_vp);

    // Each file goes straight into the working file, so a merge is bounded by the card, not RAM.
    WorkWriter* w = work_begin(storage);
    bool built = w != NULL;
    char pathbuf[MERGE_PATH_LEN];
    size_t off = 0;
    for(int i = 0; built && i < n; i++) {
        size_t plen = pbuf[off++];
        memcpy(pathbuf, pbuf + off, plen);
        pathbuf[plen] = '\0';
        off += plen;
        g_custom_btn_id = pbuf[off++];

        // A file that fails to load is skipped, as before
        work_add_sub(storage, pathbuf, w, &app->sd, i ? MergeJoinManual : MergeJoinNone);
        built = !w->fail;
    }

    built = w && work_end(w, &app->sd) && built;

    // The merge keeps the first file's settings
    app->sd.frequency = freq;
    strncpy(app->sd.preset, preset, sizeof(app->sd.preset) - 1);

    gui_remove_view_port(gui, load_vp);
    free(pbuf);

    strncpy(app->basename, "merged", sizeof(app->basename) - 1);

    if(!built)
        show_message(dialogs, "Merge failed", "Out of RAM or\nSD card error.", 32);
    else if(open_work(dialogs, app)) {
        ViewPort* vp = view_port_alloc();
        FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(InputEvent));
        view_port_draw_callback_set(vp, draw_cb, app);
        view_port_input_callback_set(vp, input_cb, queue);
        gui_add_view_port(gui, vp, GuiLayerFullscreen);

        run_editor_session(gui, vp, queue, storage, dialogs, app, false);

        gui_remove_view_port(gui, vp);
        view_port_free(vp);
        furi_message_queue_free(queue);
    }

cleanup:
    view_port_free(load_vp);
    furi_record_close(RECORD_GUI);

    if(app->sd.data) free(app->sd.data);

    free(app->undo);
    work_cleanup(storage);
    furi_mutex_free(app->mutex);
    free(app);
}
