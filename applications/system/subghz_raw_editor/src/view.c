// The waveform view: markers, zoom, auto-select, drawing and input.

#include "app.h"

void fmt_time(int32_t us, char* out, size_t n) {
    if(us < 0) us = 0;

    if(us < 1000) {
        snprintf(out, n, "%luus", (unsigned long)us);
    } else if(us < 1000000) {
        long ms = us / 1000;
        long t = (us % 1000) / 100;
        snprintf(out, n, "%ld.%ldms", ms, t);
    } else {
        long s = us / 1000000;
        long h = (us % 1000000) / 10000;
        snprintf(out, n, "%ld.%02lds", s, h);
    }
}

static int time_to_x(App* a, int32_t t) {
    int32_t span = a->view_end - a->view_start;
    if(span <= 0) span = 1;

    return (int)(((int64_t)(t - a->view_start) * SCREEN_W_PX) / span);
}

// Markers stay in the RAM window.
void clamp_marker(App* a, int idx) {
    int32_t* m = idx ? &a->marker_b : &a->marker_a;
    int32_t lo = a->sd.win_t0, hi = lo + a->sd.win_us;
    if(*m < lo) *m = lo;
    if(*m > hi) *m = hi;
}

/* How far the view may reach: the RAM window, plus a margin past the signal's
 * real start and end. */
static int32_t view_bounds(const App* a, int32_t* lo, int32_t* hi) {
    const SubData* sd = &a->sd;
    int32_t w = sd->win_us > 0 ? sd->win_us : 1;
    int32_t margin = w / 3 + 50000;
    *lo = sd->win_s0 ? sd->win_t0 : -margin;
    *hi = sd->win_t0 + w + (sd->win_s0 + sd->count < sd->file_count ? 0 : margin);
    return *hi - *lo;
}

void clamp_view(App* a) {
    int32_t lo, hi;
    int32_t fullspan = view_bounds(a, &lo, &hi);

    int32_t span = a->view_end - a->view_start;
    if(span < 200) span = 200;

    if(span > fullspan) span = fullspan;

    if(a->view_start < lo) a->view_start = lo;

    a->view_end = a->view_start + span;

    if(a->view_end > hi) {
        a->view_end = hi;
        a->view_start = hi - span;
        if(a->view_start < lo) a->view_start = lo;
    }
}

void ensure_visible(App* a, int32_t m) {
    int32_t span = a->view_end - a->view_start;
    if(m < a->view_start) {
        a->view_start = m - span / 5;
        a->view_end = a->view_start + span;
    } else if(m > a->view_end) {
        a->view_end = m + span / 5;
        a->view_start = a->view_end - span;
    }

    clamp_view(a);
}

// True when sample i is a real level change (an edge). The first sample is an
// edge only if it starts high; runs of same-sign samples - e.g. a long gap
// split into several silence samples - are one level, not repeated edges.
static bool sample_is_edge(const SubData* sd, size_t i) {
    return (i == 0) ? (sd->data[i] > 0) : ((sd->data[i] < 0) != (sd->data[i - 1] < 0));
}

void recompute_activity(App* a) {
    memset(a->activity, 0, sizeof(a->activity));
    int32_t span = a->view_end - a->view_start;
    if(span <= 0) span = 1;

    a->wave_mode = (span <= 30000);

    int32_t run = a->sd.win_t0;
    uint16_t mx = 0;
    for(size_t i = 0; i < a->sd.count; i++) {
        int32_t ad = iabs32(a->sd.data[i]);
        int32_t s0 = run;
        run += ad;
        int32_t s1 = run;

        if(s1 < a->view_start || s0 > a->view_end) continue;

        if(!sample_is_edge(&a->sd, i)) continue;

        int x = (int)(((int64_t)(s0 - a->view_start) * SCREEN_W_PX) / span);

        if(x < 0) x = 0;

        if(x > SCREEN_W_PX - 1) x = SCREEN_W_PX - 1;

        if(a->activity[x] < 65535) a->activity[x]++;

        if(a->activity[x] > mx) mx = a->activity[x];
    }

    a->act_max = mx ? mx : 1;
}

/* Select the frame start..end and frame the view around it. B extends into
 * the trailing inter-frame gap so the selection keeps the end-of-frame
 * silence that decoders (KeeLoq, ...) require to finalize a frame. Without it
 * the selection ends on the last bit and the saved capture won't decode (it
 * had to be widened by hand). Any positive extension is enough:
 * write_selection emits the whole gap sample once the marker lands inside it.
 * Limit it to GAP_KEEP_MAX_US so the marker/view stay tidy and never reach
 * into the next frame. */
__attribute__((noinline)) void select_frame(App* a, int32_t start, int32_t end, int32_t gap) {
    if(gap > GAP_KEEP_MAX_US) gap = GAP_KEEP_MAX_US;

    int32_t pad = (end - start) / 6 + 500;
    a->marker_a = start;
    a->marker_b = end + gap;
    a->view_start = start - pad * 3;
    a->view_end = a->marker_b + pad * 3;
    clamp_view(a);
}

// Runs on a fresh window, which starts the signal.
void auto_detect(App* a) {
    int32_t total = a->sd.win_us;

    if(a->sd.synthesized) {
        a->marker_a = 0;
        a->marker_b = total;
        a->view_start = 0;
        a->view_end = total > 0 ? total : 1;
        clamp_view(a);

        return;
    }

    if(total < 1 || a->sd.count < 2) {
        a->view_start = 0;
        a->view_end = total > 0 ? total : 1;
        a->marker_a = total / 4;
        a->marker_b = (total * 3) / 4;

        return;
    }

    const int32_t GAP_US = 15000;
    const int32_t MIN_FRAME_US = 30000;

    int32_t run = 0;
    int32_t seg_start = -1;
    int32_t last_edge = 0;
    int32_t best_a = -1, best_b = -1;
    int32_t best_len = 0;
    int32_t best_gap = 0;

    for(size_t i = 0; i < a->sd.count; i++) {
        int32_t ad = iabs32(a->sd.data[i]);
        int32_t s0 = run;
        run += ad;
        int32_t s1 = run;

        if(ad >= GAP_US) {
            if(seg_start >= 0) {
                int32_t seglen = last_edge - seg_start;
                if(seglen >= MIN_FRAME_US && seglen > best_len) {
                    best_len = seglen;
                    best_a = seg_start;
                    best_b = last_edge;
                    best_gap = ad;
                }

                seg_start = -1;
            }
        } else {
            if(seg_start < 0) seg_start = s0;

            last_edge = s1;
        }
    }

    if(seg_start >= 0) {
        int32_t seglen = last_edge - seg_start;
        if(seglen >= MIN_FRAME_US && seglen > best_len) {
            best_a = seg_start;
            best_b = last_edge;
            best_gap = 0;
        }
    }

    if(best_a < 0) {
        a->marker_a = 0;
        a->marker_b = total;
        a->view_start = 0;
        a->view_end = total;
        clamp_view(a);

        return;
    }

    select_frame(a, best_a, best_b, best_gap);
}

static void draw_marker(Canvas* c, int x, bool active) {
    if(x < 0 || x > SCREEN_W_PX - 1) return;

    if(active) {
        for(int y = WAVE_TOP; y <= WAVE_BOT; y++)
            canvas_draw_dot(c, x, y);

        canvas_draw_box(c, x - 1, WAVE_TOP - 4, 3, 3);
    } else {
        for(int y = WAVE_TOP; y <= WAVE_BOT; y += 2)
            canvas_draw_dot(c, x, y);
    }
}

const char busy_loading[] = "Loading...";

// Full-screen wait message, with the file name under it when there is one.
void draw_busy(Canvas* c, const char* text, const char* name) {
    canvas_clear(c);
    canvas_set_color(c, ColorBlack);
    canvas_set_font(c, FontSecondary);
    canvas_draw_str_aligned(c, 64, name ? 28 : 32, AlignCenter, AlignCenter, text);
    if(name) canvas_draw_str_aligned(c, 64, 40, AlignCenter, AlignCenter, name);
}

void draw_cb(Canvas* c, void* ctx) {
    App* a = ctx;
    furi_mutex_acquire(a->mutex, FuriWaitForever);

    if(a->busy) {
        char ln[20];
        strlcpy(ln, a->basename, sizeof(ln));
        draw_busy(c, a->busy, a->busy == busy_loading ? ln : NULL);
        furi_mutex_release(a->mutex);
        return;
    }

    canvas_clear(c);
    canvas_set_color(c, ColorBlack);
    canvas_set_font(c, FontSecondary);

    char total_time_str[14];
    fmt_time(a->sd.total_us, total_time_str, sizeof(total_time_str));
    canvas_draw_str_aligned(
        c, SCREEN_W_PX - 1, FONT_SIZE_PX, AlignRight, AlignBottom, total_time_str);

    // The name gets what the total time leaves, minus a letter to keep them apart; longer ones
    // scroll after a pause
    uint32_t elapsed = furi_get_tick() - a->opened_tick;
    size_t scroll = elapsed < NAME_SCROLL_HOLD_MS ?
                        0 :
                        1 + (elapsed - NAME_SCROLL_HOLD_MS) / NAME_SCROLL_STEP_MS;
    size_t name_w =
        SCREEN_W_PX - canvas_string_width(c, total_time_str) - canvas_glyph_width(c, 'a') - 4;
    FuriString* name = furi_string_alloc_set_str(a->basename);
    elements_scrollable_text_line(c, 0, FONT_SIZE_PX, name_w, name, scroll, false);
    furi_string_free(name);

    for(int x = 0; x < SCREEN_W_PX; x++) {
        if(a->overview[x] > a->ov_max / 6) canvas_draw_dot(c, x, OV_Y + OV_H - 1);
    }

    int32_t total = a->sd.total_us > 0 ? a->sd.total_us : 1;
    int vx0 = (int)(((int64_t)a->view_start * SCREEN_W_PX) / total);
    int vx1 = (int)(((int64_t)a->view_end * SCREEN_W_PX) / total);

    if(vx0 < 0) vx0 = 0;

    if(vx0 > SCREEN_W_PX - 1) vx0 = SCREEN_W_PX - 1;

    if(vx1 <= vx0) vx1 = vx0 + 1;

    if(vx1 > SCREEN_W_PX - 1) vx1 = SCREEN_W_PX - 1;

    canvas_draw_frame(c, vx0, OV_Y, vx1 - vx0 + 1, OV_H);
    canvas_draw_line(c, 0, SEP_Y, SCREEN_W_PX - 1, SEP_Y);

    if(a->wave_mode) {
        int yhi = WAVE_TOP + 2;
        int ylo = WAVE_BOT - 2;

        int32_t run = a->sd.win_t0;
        int prev_x = -1;
        int prev_y = ylo;

        for(size_t i = 0; i < a->sd.count; i++) {
            int32_t ad = iabs32(a->sd.data[i]);
            int32_t t0 = run;
            run += ad;
            int32_t t1 = run;

            if(t1 < a->view_start || t0 > a->view_end) continue;

            int x0 = time_to_x(a, t0);
            int x1 = time_to_x(a, t1);

            if(x0 < 0) x0 = 0;

            if(x1 > SCREEN_W_PX - 1) x1 = SCREEN_W_PX - 1;

            int y = (a->sd.data[i] > 0) ? yhi : ylo;

            if(prev_x >= 0 && y != prev_y && x0 >= 0 && x0 <= SCREEN_W_PX - 1)
                canvas_draw_line(c, x0, yhi, x0, ylo);

            if(x1 >= x0) canvas_draw_line(c, x0, y, x1, y);

            prev_x = x1;
            prev_y = y;
        }
    } else {
        for(int x = 0; x < SCREEN_W_PX; x++) {
            if(a->activity[x] == 0) continue;

            int h = (int)(((int32_t)a->activity[x] * (WAVE_BOT - WAVE_TOP - 1)) / a->act_max);
            if(h < 1) h = 1;

            canvas_draw_line(c, x, WAVE_BOT, x, WAVE_BOT - h);
        }
    }

    int xa = time_to_x(a, a->marker_a);
    int xb = time_to_x(a, a->marker_b);
    int xlo = xa < xb ? xa : xb;
    int xhi = xa < xb ? xb : xa;

    if(xlo < 0) xlo = 0;

    if(xhi > SCREEN_W_PX - 1) xhi = SCREEN_W_PX - 1;

    for(int x = xlo; x <= xhi; x += 2)
        canvas_draw_dot(c, x, WAVE_TOP - 1);

    draw_marker(c, time_to_x(a, a->marker_a), a->active == 0);
    draw_marker(c, time_to_x(a, a->marker_b), a->active == 1);

    if(a->view_start > 0) {
        canvas_draw_str_aligned(c, 0, (WAVE_TOP + WAVE_BOT) / 2, AlignLeft, AlignCenter, "<");
    }

    if(a->view_end < a->sd.total_us) {
        canvas_draw_str_aligned(
            c, SCREEN_W_PX - 1, (WAVE_TOP + WAVE_BOT) / 2, AlignRight, AlignCenter, ">");
    }

    char sa[14], sb[14], zb[14], sl[14];
    fmt_time(a->marker_a, sa, sizeof(sa));
    fmt_time(a->marker_b, sb, sizeof(sb));

    int32_t len = a->marker_b - a->marker_a;
    if(len < 0) len = -len;

    int32_t vis_lo = a->view_start < 0 ? 0 : a->view_start;
    int32_t vis_hi = a->view_end > a->sd.total_us ? a->sd.total_us : a->view_end;
    int32_t vis = vis_hi - vis_lo;
    if(vis < 0) vis = 0;

    fmt_time(vis, zb, sizeof(zb));
    fmt_time(len, sl, sizeof(sl));

    char l1[SCREEN_W_PX / 4];
    char l2[SCREEN_W_PX / 4];

    snprintf(l1, sizeof(l1), "%sA: %s", a->active == 0 ? ">" : "  ", sa);
    snprintf(l2, sizeof(l2), "%sB: %s", a->active == 1 ? ">" : "  ", sb);
    canvas_draw_str(c, 0, LINE1_BASE, l1);
    canvas_draw_str(c, 0, LINE2_BASE, l2);

    snprintf(l1, sizeof(l1), "View: %s", zb);
    snprintf(l2, sizeof(l2), "Marked: %s", sl);
    canvas_draw_str_aligned(c, SCREEN_W_PX - 1, 56, AlignRight, AlignBottom, l1);
    canvas_draw_str_aligned(
        c, SCREEN_W_PX - 1, 56 + FONT_SIZE_PX + 1, AlignRight, AlignBottom, l2);

    if(a->mode != EditModeMenu && a->status[0] && furi_get_tick() < a->status_until) {
        int w = 124;
        canvas_set_color(c, ColorWhite);
        canvas_draw_box(c, 2, 22, w, 22);
        canvas_set_color(c, ColorBlack);
        canvas_draw_frame(c, 2, 22, w, 22);

        // One line, or two split at '\n'
        char* nl = strchr(a->status, '\n');
        if(nl) {
            *nl = '\0';
            canvas_draw_str_aligned(c, 64, 29, AlignCenter, AlignCenter, a->status);
            canvas_draw_str_aligned(c, 64, 38, AlignCenter, AlignCenter, nl + 1);
            *nl = '\n';
        } else {
            canvas_draw_str_aligned(c, 64, 33, AlignCenter, AlignCenter, a->status);
        }
    }

    if(a->mode == EditModeMenu) {
        const int bx = 4, bw = 52, bh = MENU_ITEMS * 14 + 4, by = (SCREEN_H_PX - bh) / 2;
        canvas_set_color(c, ColorWhite);
        canvas_draw_box(c, bx, by, bw, bh);
        canvas_set_color(c, ColorBlack);
        canvas_draw_frame(c, bx, by, bw, bh);

        // Read-only A..B details on the right.
        const int px = bx + bw + 3, pw = SCREEN_W_PX - 2 - px, ph = 3 * 10 + 6,
                  py = (SCREEN_H_PX - ph) / 2;
        canvas_set_color(c, ColorWhite);
        canvas_draw_box(c, px, py, pw, ph);
        canvas_set_color(c, ColorBlack);
        canvas_draw_frame(c, px, py, pw, ph);

        char info[3][20];
        snprintf(info[0], sizeof(info[0]), "Spl: %lu", (unsigned long)a->info_spl);
        if(a->info_te)
            snprintf(info[1], sizeof(info[1]), "TE: %luus", (unsigned long)a->info_te);
        else
            snprintf(info[1], sizeof(info[1]), "TE: -");
        snprintf(info[2], sizeof(info[2]), "Enc: %s", a->info_enc ? a->info_enc : "-");
        for(int i = 0; i < 3; i++)
            canvas_draw_str(c, px + 4, py + 3 + FONT_SIZE_PX + i * 10, info[i]);

        static const char* const ops[EditOps] = {"Cut", "Copy", "Paste"};
        const char* labels[MENU_ITEMS] = {"Save", "Decode", ops[a->edit_op], "Undo"};
        for(int i = 0; i < MENU_ITEMS; i++) {
            int ry = by + 2 + i * 14;
            int rh = 13;
            bool disabled = (i == MENU_SEL_UNDO && !a->undo_n) ||
                            (i == MENU_SEL_CUT && a->edit_op == EditPaste && !a->clip_count);

            if(a->menu_sel == i) {
                canvas_set_color(c, ColorBlack);
                canvas_draw_box(c, bx + 2, ry, bw - 4, rh);
                canvas_set_color(c, ColorWhite);
            } else {
                canvas_set_color(c, ColorBlack);
            }

            int cx = bx + bw / 2;
            canvas_draw_str_aligned(c, cx, ry + rh / 2 + 1, AlignCenter, AlignCenter, labels[i]);

            if(disabled) canvas_draw_line(c, cx - 13, ry + rh / 2, cx + 13, ry + rh / 2);

            // Filled arrowheads: Left/Right switch Cut, Copy and Paste
            if(i == MENU_SEL_CUT) {
                int cy = ry + rh / 2;
                for(int j = 0; j < 3; j++) {
                    canvas_draw_line(c, bx + 5 + j, cy - j, bx + 5 + j, cy + j);
                    canvas_draw_line(c, bx + bw - 6 - j, cy - j, bx + bw - 6 - j, cy + j);
                }
            }
        }

        canvas_set_color(c, ColorBlack);
    }

    furi_mutex_release(a->mutex);
}

void input_cb(InputEvent* e, void* ctx) {
    FuriMessageQueue* q = ctx;
    furi_message_queue_put(q, e, FuriWaitForever);
}

void zoom(App* a, bool in) {
    int32_t m = a->active ? a->marker_b : a->marker_a;
    int32_t span = a->view_end - a->view_start;
    int32_t nspan = in ? (span * 2 / 3) : (span * 3 / 2);
    if(nspan < 200) nspan = 200;

    int32_t lo, hi;
    int32_t fullspan = view_bounds(a, &lo, &hi);
    if(nspan > fullspan) {
        nspan = fullspan;
        if(!in && work_windowed(&a->sd)) {
            snprintf(a->status, sizeof(a->status), "No RAM to zoom out\nMove sideways");
            a->status_until = furi_get_tick() + 1500;
        }
    }

    if(nspan < 1) nspan = 1;

    int32_t off = (int32_t)(((int64_t)(m - a->view_start) * nspan) / (span > 0 ? span : 1));
    a->view_start = m - off;
    a->view_end = a->view_start + nspan;
    clamp_view(a);
}
