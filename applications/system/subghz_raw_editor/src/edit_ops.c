// A..B operations: save names, writing the selection, the Undo snapshot and Cut.

#include "app.h"

// First free name: <base> itself when bare, then <base><sep>1, <base><sep>2, ...
void propose_name(
    Storage* st,
    const char* base,
    const char* sep,
    bool bare,
    char* out,
    size_t outlen) {
    FuriString* path = furi_string_alloc();
    for(int n = bare ? 0 : 1; n < 1000; n++) {
        if(n)
            snprintf(out, outlen, "%s%s%d", base, sep, n);
        else
            strlcpy(out, base, outlen);
        furi_string_printf(path, "%s/%s.sub", SUBGHZ_DIR, out);
        if(!storage_file_exists(st, furi_string_get_cstr(path))) break;
    }

    furi_string_free(path);
}

// <name>_editN for a RAW save; an open <name>_editK continues the same series.
void propose_edit_name(Storage* st, const App* a, char* out, size_t outlen) {
    char base[sizeof(a->basename)];
    strlcpy(base, a->basename, sizeof(base));

    char* hit = NULL;
    for(char* s = base; (s = strstr(s, "_edit")); s++)
        hit = s;

    if(hit) {
        const char* t = hit + 5;
        while(*t >= '0' && *t <= '9')
            t++;
        if(!*t) *hit = '\0';
    }

    propose_name(st, base, "_edit", false, out, outlen);
}

// Sample indices touched by A..B. False when nothing falls inside.
bool selection_range(const App* a, size_t* i0, size_t* i1) {
    int32_t lo = a->marker_a, hi = a->marker_b;
    if(lo > hi) {
        int32_t t = lo;
        lo = hi;
        hi = t;
    }

    int32_t run = a->sd.win_t0;
    bool started = false;
    for(size_t i = 0; i < a->sd.count; i++) {
        int32_t ad = iabs32(a->sd.data[i]);
        int32_t s0 = run;
        run += ad;
        int32_t s1 = run;
        if(s1 > lo && s0 < hi) {
            if(!started) {
                *i0 = i;
                started = true;
            }
            *i1 = i;
        }
    }

    return started;
}

bool write_selection(Storage* st, App* a, const char* path) {
    size_t i0 = 0, i1 = 0;
    if(!selection_range(a, &i0, &i1)) {
        snprintf(a->status, sizeof(a->status), "Nothing in A..B");
        return false;
    }

    /* A synthesized frame is exact - keep its leading guard (a low gap) and
     * trailing guard verbatim. The trims below would strip the leading silence
     * (making preamble-less protocols like Princeton undecodable) so skip them. */
    if(!a->sd.synthesized) {
        while(i0 < i1 && a->sd.data[i0] < 0)
            i0++;

        while(i1 > i0 && a->sd.data[i1] > 0)
            i1--;
    }

    if(i0 >= i1) {
        snprintf(a->status, sizeof(a->status), "Range too small");
        return false;
    }

    File* f = storage_file_alloc(st);
    if(!storage_file_open(f, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        storage_file_free(f);
        snprintf(a->status, sizeof(a->status), "Write failed");
        return false;
    }

    char hdr[192];
    int n = snprintf(
        hdr,
        sizeof(hdr),
        "Filetype: Flipper SubGhz RAW File\nVersion: 1\nFrequency: %lu\nPreset: %s\nProtocol: RAW\n",
        (unsigned long)a->sd.frequency,
        a->sd.preset);

    storage_file_write(f, hdr, n);

    FuriString* lbuf = furi_string_alloc();
    furi_string_set(lbuf, "RAW_Data:");

    int cnt = 0;
    for(size_t i = i0; i <= i1; i++) {
        char vb[16];
        snprintf(vb, sizeof(vb), " %ld", (long)a->sd.data[i]);
        furi_string_cat_str(lbuf, vb);
        if(++cnt >= 512) {
            furi_string_push_back(lbuf, '\n');
            storage_file_write(f, furi_string_get_cstr(lbuf), furi_string_size(lbuf));
            furi_string_set(lbuf, "RAW_Data:");
            cnt = 0;
        }
    }

    if(cnt > 0) {
        furi_string_push_back(lbuf, '\n');
        storage_file_write(f, furi_string_get_cstr(lbuf), furi_string_size(lbuf));
    }

    furi_string_free(lbuf);
    storage_file_close(f);
    storage_file_free(f);
    return true;
}

// Samples left once A..B is cut out; only with apply is the buffer rewritten in place.
size_t cut_run(App* a, bool apply) {
    int32_t lo = a->marker_a, hi = a->marker_b;
    if(lo > hi) {
        int32_t t = lo;
        lo = hi;
        hi = t;
    }

    int16_t* d = a->sd.data;
    size_t n = a->sd.count;
    size_t w = 0;
    int32_t run = a->sd.win_t0;
    int32_t last = 0; // d[w - 1], tracked so a dry run needs no writes
    bool seam = false; // true once something was dropped before the current sample
    for(size_t i = 0; i < n; i++) {
        int32_t v = d[i];
        int32_t ad = iabs32(v);
        int32_t s0 = run;
        run += ad;
        int32_t s1 = run;
        int sign = (v >= 0) ? 1 : -1;
        int32_t keep = 0;

        if(s0 < lo) keep += (s1 < lo ? s1 : lo) - s0;

        if(s1 > hi) keep += s1 - (s0 > hi ? s0 : hi);

        if(keep < ad) seam = true; // part (or all) of this sample falls inside A..B

        if(keep <= 0) continue;

        int32_t val = sign * keep;
        // Merge only across the cut seam. Consecutive same-sign samples elsewhere
        // are long gaps split by work_put() and must stay as they are.
        if(seam && w > 0 && ((last >= 0) == (val >= 0))) {
            int32_t sum = last + val;
            if(iabs32(sum) <= DUR_CLAMP) {
                last = sum;
                if(apply) d[w - 1] = (int16_t)sum;
            } else {
                // Keep the full duration: fill the previous sample, spill the rest.
                // Safe in place: at most one extra sample, written at index <= i.
                last = sum - sign * DUR_CLAMP;
                if(apply) {
                    d[w - 1] = (int16_t)(sign * DUR_CLAMP);
                    d[w] = (int16_t)last;
                }
                w++;
            }
        } else {
            last = val;
            if(apply) d[w] = (int16_t)val;
            w++;
        }

        if(keep == ad && s0 >= hi) seam = false; // past the cut, back to verbatim copying
    }

    if(apply) a->sd.count = w;
    return w;
}

/* Cut A..B out of the window in place; the caller has checked with cut_run()
 * that it leaves a signal, and writes the result to the working file. */
void cut_compact(App* a) {
    int32_t lo = a->marker_a < a->marker_b ? a->marker_a : a->marker_b;

    cut_run(a, true);
    a->sd.win_us = samples_us(a->sd.data, a->sd.count);

    a->marker_a = lo;
    a->marker_b = lo;
    clamp_marker(a, 0);
    clamp_marker(a, 1);
    a->active = 0;

    int32_t span = a->view_end - a->view_start;
    if(span < 200) span = 200;

    a->view_start = a->marker_a - span / 2;
    a->view_end = a->view_start + span;
}
