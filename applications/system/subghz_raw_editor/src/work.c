/* The RAM window over the working file: loading and sliding it, the overview of
 * the whole signal, and Cut and Undo, which rewrite the file. A cut keeps the
 * file it replaced as WORK_UNDO_PATH, so Undo costs no RAM. */

#include "app.h"

int32_t samples_us(const int16_t* d, size_t n) {
    int64_t t = 0;
    for(size_t i = 0; i < n; i++)
        t += iabs32(d[i]);

    return t > 0x7FFFFFFF ? 0x7FFFFFFF : (int32_t)t;
}

// More of the signal than the window holds.
bool work_windowed(const SubData* sd) {
    return sd->win_s0 || sd->win_s0 + sd->count < sd->file_count;
}

static File* work_file(Storage* st, const char* path, FS_AccessMode mode, FS_OpenMode open) {
    File* f = storage_file_alloc(st);
    if(storage_file_open(f, path, mode, open)) return f;

    storage_file_free(f);
    return NULL;
}

static void work_close(File* f) {
    storage_file_close(f);
    storage_file_free(f);
}

static bool work_read(File* f, size_t s0, int16_t* dst, size_t n) {
    size_t bytes = n * sizeof(int16_t);
    return storage_file_seek(f, s0 * sizeof(int16_t), true) &&
           storage_file_read(f, dst, bytes) == bytes;
}

// Read n samples from s0 of the working file into dst.
static bool work_read_at(App* a, size_t s0, int16_t* dst, size_t n) {
    File* f = work_file(a->st, WORK_PATH, FSAM_READ, FSOM_OPEN_EXISTING);
    if(!f) return false;

    bool ok = work_read(f, s0, dst, n);
    work_close(f);
    return ok;
}

// Fill the window from sample s0; the caller sets win_t0.
bool work_load(App* a, size_t s0) {
    SubData* sd = &a->sd;
    size_t n = sd->file_count - s0;
    if(n > sd->cap) n = sd->cap;

    bool ok = work_read_at(a, s0, sd->data, n);
    sd->win_s0 = s0;
    sd->count = ok ? n : 0;
    sd->win_us = samples_us(sd->data, sd->count);
    return ok;
}

/* Move the window a quarter of itself later (dir > 0) or earlier. The kept
 * part shifts in RAM and only the new quarter is read. False at either end of
 * the signal. */
bool work_slide(App* a, int dir) {
    SubData* sd = &a->sd;
    size_t k = sd->cap / 4 ? sd->cap / 4 : 1;
    size_t end = sd->win_s0 + sd->count;
    bool ok;
    if(dir > 0) {
        if(end >= sd->file_count) return false;

        if(k > sd->file_count - end) k = sd->file_count - end;

        sd->win_t0 += samples_us(sd->data, k);
        memmove(sd->data, sd->data + k, (sd->count - k) * sizeof(int16_t));
        ok = work_read_at(a, end, sd->data + sd->count - k, k);
        sd->win_s0 += k;
    } else {
        if(!sd->win_s0) return false;

        if(k > sd->win_s0) k = sd->win_s0;

        memmove(sd->data + k, sd->data, (sd->count - k) * sizeof(int16_t));
        ok = work_read_at(a, sd->win_s0 - k, sd->data, k);
        sd->win_s0 -= k;
        sd->win_t0 -= samples_us(sd->data, k);
    }

    if(!ok) work_load(a, sd->win_s0);

    sd->win_us = samples_us(sd->data, sd->count);
    return ok;
}

// The overview bar covers the whole signal, so it is read from the file.
void recompute_overview(App* a) {
    memset(a->overview, 0, sizeof(a->overview));
    int32_t total = a->sd.total_us > 0 ? a->sd.total_us : 1;
    uint16_t mx = 0;

    File* f = work_file(a->st, WORK_PATH, FSAM_READ, FSOM_OPEN_EXISTING);
    if(f) {
        int16_t buf[WORK_CHUNK];
        int64_t t = 0;
        bool prev_low = true; // the first sample is an edge when it is a pulse
        size_t got;
        while((got = storage_file_read(f, buf, sizeof(buf)) / sizeof(int16_t)) > 0) {
            for(size_t i = 0; i < got; i++) {
                bool low = buf[i] < 0;
                if(low != prev_low) {
                    int x = (int)((t * SCREEN_W_PX) / total);
                    if(x > SCREEN_W_PX - 1) x = SCREEN_W_PX - 1;

                    if(a->overview[x] < UINT16_MAX) a->overview[x]++;

                    if(a->overview[x] > mx) mx = a->overview[x];
                }

                prev_low = low;
                t += iabs32(buf[i]);
            }
        }

        work_close(f);
    }

    a->ov_max = mx ? mx : 1;
}

/* Snap the jitter of the whole file to the TE fitted on the first window,
 * rewriting it in place (same sample count), then reload the window. */
static bool work_jitter(App* a) {
    SubData* sd = &a->sd;
    float te = jitter_te(sd->data, sd->count);
    if(te <= 0.0f) return true;

    File* f = work_file(a->st, WORK_PATH, FSAM_READ_WRITE, FSOM_OPEN_EXISTING);
    if(!f) return false;

    int16_t buf[WORK_CHUNK];
    int64_t total = 0;
    bool ok = true;
    for(size_t s = 0; ok && s < sd->file_count; s += WORK_CHUNK) {
        size_t n = sd->file_count - s < WORK_CHUNK ? sd->file_count - s : WORK_CHUNK;
        size_t bytes = n * sizeof(int16_t);
        ok = work_read(f, s, buf, n);
        jitter_apply(buf, n, te);
        total += samples_us(buf, n);
        ok = ok && storage_file_seek(f, s * sizeof(int16_t), true) &&
             storage_file_write(f, buf, bytes) == bytes;
    }

    work_close(f);
    sd->total_us = total > 0x7FFFFFFF ? 0x7FFFFFFF : (int32_t)total;
    return ok && work_load(a, 0);
}

/* Take the RAM window once the working file is written: all of the signal when
 * it fits, else as much as RAM allows. */
bool work_open(App* a) {
    SubData* sd = &a->sd;

    // The Undo history comes first: once the window is in, RAM is down to the reserve
    a->undo = safe_malloc(UNDO_MAX * sizeof(UndoEntry));
    if(!a->undo) {
        sd->out_of_memory = true;
        return false;
    }

    size_t cap = sample_budget();
    if(cap > sd->file_count) cap = sd->file_count;

    sd->data = cap >= 2 ? safe_malloc(cap * sizeof(int16_t)) : NULL;
    if(!sd->data) {
        sd->out_of_memory = true;
        return false;
    }

    sd->cap = cap;
    sd->win_t0 = 0;
    if(!work_load(a, 0) || (g_normalize_jitter && !work_jitter(a))) return false;

    recompute_overview(a);
    return true;
}

static void state_save(const App* a, WorkState* s) {
    *s = (WorkState){
        .file_count = a->sd.file_count,
        .win_s0 = a->sd.win_s0,
        .win_t0 = a->sd.win_t0,
        .total_us = a->sd.total_us,
        .marker_a = a->marker_a,
        .marker_b = a->marker_b,
        .view_start = a->view_start,
        .view_end = a->view_end,
        .active = a->active,
    };
}

static void state_restore(App* a, const WorkState* s) {
    a->sd.file_count = s->file_count;
    a->sd.win_t0 = s->win_t0;
    a->sd.total_us = s->total_us;
    a->marker_a = s->marker_a;
    a->marker_b = s->marker_b;
    a->view_start = s->view_start;
    a->view_end = s->view_end;
    a->active = s->active;
    work_load(a, s->win_s0);
}

static void undo_path(char* out, size_t n, uint32_t id) {
    snprintf(out, n, UNDO_DIR "/undo_%lu.bin", (unsigned long)id);
}

/* Make WORK_NEW_PATH the working file, keeping the old one as a new step of
 * the Undo history; past UNDO_MAX steps the oldest is dropped. */
static bool work_commit(App* a, const WorkState* before, EditOp op, int32_t spl, int32_t at) {
    char path[48];
    if(a->undo_n == UNDO_MAX) {
        undo_path(path, sizeof(path), a->undo[0].id);
        storage_simply_remove(a->st, path);
        memmove(a->undo, a->undo + 1, --a->undo_n * sizeof(UndoEntry));
    }

    undo_path(path, sizeof(path), a->undo_seq);
    storage_simply_mkdir(a->st, UNDO_DIR);
    if(storage_common_rename(a->st, WORK_PATH, path) != FSE_OK) return false;

    if(storage_common_rename(a->st, WORK_NEW_PATH, WORK_PATH) != FSE_OK) {
        storage_common_rename(a->st, path, WORK_PATH);
        return false;
    }

    a->undo[a->undo_n++] =
        (UndoEntry){.st = *before, .id = a->undo_seq++, .spl = spl, .at_us = at, .op = op};
    return true;
}

static bool work_copy(File* src, File* dst, size_t from, size_t n) {
    int16_t buf[WORK_CHUNK];
    if(!storage_file_seek(src, from * sizeof(int16_t), true)) return false;

    while(n) {
        size_t k = n < WORK_CHUNK ? n : WORK_CHUNK;
        size_t bytes = k * sizeof(int16_t);
        if(storage_file_read(src, buf, bytes) != bytes ||
           storage_file_write(dst, buf, bytes) != bytes)
            return false;

        n -= k;
    }

    return true;
}

/* Cut A..B out of the window, then write the file anew: what precedes the
 * window, the window, what follows it. The old file becomes the Undo copy. On
 * a card error nothing changes. */
bool work_cut(App* a) {
    SubData* sd = &a->sd;
    WorkState before;
    state_save(a, &before);
    size_t old_count = sd->count;
    int32_t old_us = sd->win_us;

    cut_compact(a);

    bool ok = false;
    File* src = work_file(a->st, WORK_PATH, FSAM_READ, FSOM_OPEN_EXISTING);
    File* dst = work_file(a->st, WORK_NEW_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS);
    if(src && dst) {
        size_t bytes = sd->count * sizeof(int16_t);
        size_t after = sd->win_s0 + old_count;
        ok = work_copy(src, dst, 0, sd->win_s0) &&
             storage_file_write(dst, sd->data, bytes) == bytes &&
             work_copy(src, dst, after, sd->file_count - after);
    }

    if(src) work_close(src);

    if(dst) work_close(dst);

    if(!ok ||
       !work_commit(a, &before, EditCut, (int32_t)sd->count - (int32_t)old_count, a->marker_a)) {
        storage_simply_remove(a->st, WORK_NEW_PATH);
        state_restore(a, &before);
        recompute_activity(a);
        return false;
    }

    sd->file_count = before.file_count - old_count + sd->count;
    sd->total_us = before.total_us - (old_us - sd->win_us);
    work_load(a, sd->win_s0); // refill what the cut emptied
    clamp_view(a);
    recompute_overview(a);
    recompute_activity(a);
    return true;
}

static bool put(File* f, const int16_t* d, size_t n) {
    size_t bytes = n * sizeof(int16_t);
    return !n || storage_file_write(f, d, bytes) == bytes;
}

// Copy exactly A..B - samples cut by a marker keep only their part inside - to CLIP_PATH.
bool clip_store(App* a) {
    SubData* sd = &a->sd;
    size_t i0, i1;
    if(!selection_range(a, &i0, &i1)) return false;

    int32_t lo = a->marker_a < a->marker_b ? a->marker_a : a->marker_b;
    int32_t hi = a->marker_a < a->marker_b ? a->marker_b : a->marker_a;
    File* f = work_file(a->st, CLIP_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS);
    if(!f) return false;

    int16_t buf[WORK_CHUNK];
    size_t n = 0, count = 0;
    int32_t us = 0;
    int32_t t = sd->win_t0 + samples_us(sd->data, i0);
    bool ok = true;
    for(size_t i = i0; i <= i1; i++) {
        int32_t s0 = t;
        t += iabs32(sd->data[i]);
        int32_t keep = (t < hi ? t : hi) - (s0 > lo ? s0 : lo);
        if(keep <= 0) continue;

        buf[n++] = (int16_t)(sd->data[i] < 0 ? -keep : keep);
        count++;
        us += keep;
        if(n == WORK_CHUNK) {
            ok = ok && put(f, buf, n);
            n = 0;
        }
    }

    ok = ok && put(f, buf, n);
    work_close(f);
    a->clip_count = ok ? count : 0;
    a->clip_us = ok ? us : 0;
    return ok && count;
}

/* Writes samples joining runs of one sign and re-splitting them as loading
 * does (DUR_CLAMP pieces, then the rest), so a paste undoes a cut's seam to
 * the very sample. */
typedef struct {
    File* f;
    int32_t run; // the pending run, signed
    bool any;
    bool ok;
    size_t count;
    size_t n;
    int16_t buf[WORK_CHUNK];
} Joiner;

static void join_out(Joiner* j, int32_t v) {
    if(j->n == WORK_CHUNK) {
        j->ok = j->ok && put(j->f, j->buf, j->n);
        j->n = 0;
    }

    j->buf[j->n++] = (int16_t)v;
    j->count++;
}

static void join_emit(Joiner* j) {
    if(!j->any) return;

    int32_t sign = j->run < 0 ? -1 : 1, mag = iabs32(j->run);
    for(; mag > DUR_CLAMP; mag -= DUR_CLAMP)
        join_out(j, sign * DUR_CLAMP);

    join_out(j, sign * mag);
    j->any = false;
}

static void join_put(Joiner* j, const int16_t* d, size_t n) {
    for(size_t i = 0; i < n; i++) {
        if(j->any && (d[i] < 0) != (j->run < 0)) join_emit(j);

        j->run = j->any ? j->run + d[i] : d[i];
        j->any = true;
    }
}

// Append n samples of src from sample from.
static void join_copy(Joiner* j, File* src, size_t from, size_t n) {
    int16_t buf[WORK_CHUNK];
    j->ok = j->ok && storage_file_seek(src, from * sizeof(int16_t), true);
    while(j->ok && n) {
        size_t k = n < WORK_CHUNK ? n : WORK_CHUNK;
        j->ok = storage_file_read(src, buf, k * sizeof(int16_t)) == k * sizeof(int16_t);
        join_put(j, buf, k);
        n -= k;
    }
}

static bool join_end(Joiner* j) {
    join_emit(j);
    j->ok = j->ok && put(j->f, j->buf, j->n);
    j->n = 0;
    return j->ok;
}

// Grow the window to hold a signal that a paste made longer, as far as RAM allows.
static void window_fit(App* a) {
    SubData* sd = &a->sd;
    size_t block = memmgr_heap_get_max_free_block();
    size_t ncap = block > LOAD_HEAP_RESERVE ? (block - LOAD_HEAP_RESERVE) / sizeof(int16_t) : 0;
    if(ncap > sd->file_count) ncap = sd->file_count;

    if(ncap <= sd->cap) return;

    int16_t* grown = safe_realloc(sd->data, ncap * sizeof(int16_t));
    if(grown) {
        sd->data = grown;
        sd->cap = ncap;
    }
}

/* Insert the clipboard at the active marker, splitting the sample under it,
 * and select what was pasted. The file is written anew like for a cut; the
 * window and the clipboard go through a Joiner, so the seams come out as
 * loading would write them. */
bool work_paste(App* a) {
    SubData* sd = &a->sd;
    int32_t at = a->active ? a->marker_b : a->marker_a;
    size_t i = 0;
    int32_t s0 = sd->win_t0;
    while(i < sd->count && s0 + iabs32(sd->data[i]) <= at)
        s0 += iabs32(sd->data[i++]);

    // data[i] runs over the marker: its head goes before the clipboard, the rest after
    int16_t split[2];
    bool cut_sample = i < sd->count && at > s0;
    if(cut_sample) {
        int32_t sign = sd->data[i] < 0 ? -1 : 1;
        split[0] = (int16_t)(sign * (at - s0));
        split[1] = (int16_t)(sd->data[i] - split[0]);
    }

    size_t rest = i + cut_sample;
    WorkState before;
    state_save(a, &before);
    bool ok = false;
    size_t after = sd->win_s0 + sd->count;
    Joiner* j = malloc(sizeof(Joiner)); // brief, so from the reserve kept for such
    File* src = work_file(a->st, WORK_PATH, FSAM_READ, FSOM_OPEN_EXISTING);
    File* clip = work_file(a->st, CLIP_PATH, FSAM_READ, FSOM_OPEN_EXISTING);
    File* dst = work_file(a->st, WORK_NEW_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS);
    if(j && src && clip && dst && work_copy(src, dst, 0, sd->win_s0)) {
        *j = (Joiner){.f = dst, .ok = true};
        join_put(j, sd->data, i);
        join_put(j, split, cut_sample);
        join_copy(j, clip, 0, a->clip_count);
        join_put(j, split + 1, cut_sample);
        join_put(j, sd->data + rest, sd->count - rest);
        ok = join_end(j) && work_copy(src, dst, after, sd->file_count - after);
    }

    // Samples now in place of the window's
    size_t win_new = ok ? j->count : 0;
    free(j);

    if(src) work_close(src);

    if(clip) work_close(clip);

    if(dst) work_close(dst);

    if(!ok || !work_commit(a, &before, EditPaste, (int32_t)a->clip_count, at)) {
        storage_simply_remove(a->st, WORK_NEW_PATH);
        return false;
    }

    sd->file_count += win_new - sd->count;
    sd->total_us += a->clip_us;
    window_fit(a);
    work_load(a, sd->win_s0);
    a->marker_a = at;
    a->marker_b = at + a->clip_us;
    clamp_marker(a, 1);
    ensure_visible(a, a->active ? a->marker_b : a->marker_a);
    recompute_overview(a);
    recompute_activity(a);
    return true;
}

/* Go back to the state before step k of the Undo history, dropping it and
 * every later step. */
bool work_undo(App* a, size_t k) {
    char path[48];
    if(k >= a->undo_n) return false;

    undo_path(path, sizeof(path), a->undo[k].id);
    if(storage_common_rename(a->st, path, WORK_PATH) != FSE_OK) return false;

    for(size_t i = k + 1; i < a->undo_n; i++) {
        undo_path(path, sizeof(path), a->undo[i].id);
        storage_simply_remove(a->st, path);
    }

    a->undo_n = k;
    state_restore(a, &a->undo[k].st);
    recompute_overview(a);
    recompute_activity(a);
    return true;
}

// "3. Cut -120 @1.25s": the step's number, what it did and where.
void undo_label(const App* a, size_t k, char* out, size_t n) {
    const UndoEntry* e = &a->undo[k];
    char at[14];
    fmt_time(e->at_us, at, sizeof(at));
    snprintf(
        out,
        n,
        "%u. %s %+ld @%s",
        (unsigned)(k + 1),
        e->op == EditPaste ? "Paste" : "Cut",
        (long)e->spl,
        at);
}

void work_cleanup(Storage* st) {
    storage_simply_remove_recursive(st, WORK_DIR);
}
