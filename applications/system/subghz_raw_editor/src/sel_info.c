/* Details of A..B for the panel beside the menu: how many samples it holds,
 * the base pulse width (TE) and the line coding its pulses suggest. A RAW
 * capture is just levels and durations, so one recording can hold several
 * remotes with different codings - this looks only at the selection. */

#include "app.h"

#define INFO_BIN_US    16
#define INFO_BINS      128 // the TE search covers runs up to 2 ms
#define INFO_MAX_UNITS 4 // longer runs are gaps or headers, not bits

typedef struct {
    const int16_t* data;
    size_t i, end;
} RunIter;

// Next level run - consecutive same-sign samples merged. False at the end.
static bool run_next(RunIter* it, bool* level, uint32_t* dur) {
    if(it->i > it->end) return false;

    *level = it->data[it->i] > 0;
    *dur = 0;
    while(it->i <= it->end && (it->data[it->i] > 0) == *level)
        *dur += iabs32(it->data[it->i++]);

    return true;
}

static uint32_t units(uint32_t dur, uint32_t te) {
    return (dur + te / 2) / te;
}

/* TE: the shortest width that recurs - the first histogram window holding an
 * eighth of the runs, climbed to its peak - then refined over every run that
 * is a small multiple of it. The refinement averages out the receiver
 * stretching highs and shrinking lows (or the reverse). */
static uint32_t info_te(const int16_t* d, size_t i0, size_t i1) {
    uint16_t h[INFO_BINS] = {0};
    uint32_t n = 0;
    RunIter it = {d, i0, i1};
    bool lv;
    uint32_t dur;
    while(run_next(&it, &lv, &dur)) {
        if(dur < INFO_BINS * INFO_BIN_US) {
            if(h[dur / INFO_BIN_US] < UINT16_MAX) h[dur / INFO_BIN_US]++;
            n++;
        }
    }

    if(n < 4) return 0;

    uint32_t need = n / 8 > 2 ? n / 8 : 2;
    int peak = -1;
    uint32_t best = 0;
    for(int k = 0; k < INFO_BINS; k++) {
        uint32_t w = h[k] + (k ? h[k - 1] : 0) + (k + 1 < INFO_BINS ? h[k + 1] : 0);
        if(peak < 0 ? w >= need : w > best) {
            peak = k;
            best = w;
        } else if(peak >= 0 && w < best)
            break;
    }

    if(peak < 0) return 0;

    /* Total time over total units: first the runs within 40% of the peak as
     * one unit each (the cluster mean), then every run, twice. A selection
     * never spans more than int32 µs, so the sums fit. */
    uint32_t te = peak * INFO_BIN_US + INFO_BIN_US / 2;
    for(int pass = 0; pass < 3; pass++) {
        uint32_t tsum = 0, usum = 0;
        it = (RunIter){d, i0, i1};
        while(run_next(&it, &lv, &dur)) {
            uint32_t u = pass ? units(dur, te) :
                                (dur < te * 2 && dur * 5 >= te * 3 && dur * 5 <= te * 7);
            if(u >= 1 && u <= INFO_MAX_UNITS) {
                tsum += dur;
                usum += u;
            }
        }

        if(usum) te = tsum / usum;
    }

    return te;
}

/* Line coding from run widths in TE units:
 *  PWM    - highs take two widths and each high+low pair lasts the same
 *  PPM    - highs take one width, lows carry the data
 *  Manch. - highs and lows both 1 or 2 TE, pair lengths vary
 *  NRZ    - other runs that are multiples of TE (e.g. wM-Bus mode C)
 *  1:1    - one width for both: a preamble or clock only */
static const char* info_enc(const int16_t* d, size_t i0, size_t i1, uint32_t te) {
    uint16_t hi[INFO_MAX_UNITS + 1] = {0}, lo[INFO_MAX_UNITS + 1] = {0};
    uint16_t sums[2 * INFO_MAX_UNITS + 1] = {0};
    uint32_t runs = 0, fit = 0, pairs = 0, pend = 0;

    RunIter it = {d, i0, i1};
    bool lv;
    uint32_t dur;
    while(run_next(&it, &lv, &dur)) {
        uint32_t u = units(dur, te);
        if(u > INFO_MAX_UNITS) {
            pend = 0;
            continue;
        }

        runs++;
        if(!u) {
            pend = 0;
            continue;
        }

        if(iabs32((int32_t)dur - (int32_t)(u * te)) * 5 <= (int32_t)(te * 2)) fit++;

        if(lv) {
            hi[u]++;
            pend = u;
        } else {
            lo[u]++;
            if(pend) {
                sums[pend + u]++;
                pairs++;
            }

            pend = 0;
        }
    }

    if(runs < 8 || fit * 10 < runs * 7) return "?";

    uint32_t hs = 0, ls = 0, top = 0, wide = 0;
    for(int u = 1; u <= INFO_MAX_UNITS; u++) {
        hs += hi[u];
        ls += lo[u];
        if(u > 2) wide += hi[u] + lo[u];
    }

    int hw = 0, lw = 0;
    for(int u = 1; u <= INFO_MAX_UNITS; u++) {
        hw += hi[u] * 8 > hs;
        lw += lo[u] * 8 > ls;
    }

    for(size_t s = 0; s < COUNT_OF(sums); s++)
        if(sums[s] > top) top = sums[s];

    if(hw >= 2 && pairs && top * 4 >= pairs * 3) return "PWM";

    if(hw == 1 && lw >= 2) return "PPM";

    if(hw >= 2 && lw >= 2 && wide * 10 < hs + ls) return "Manch.";

    if(hw == 1 && lw == 1) return "1:1";

    return "NRZ";
}

void selection_info(App* a) {
    size_t i0, i1;
    a->info_spl = 0;
    a->info_te = 0;
    a->info_enc = "-";
    if(!selection_range(a, &i0, &i1)) return;

    a->info_spl = i1 - i0 + 1;

    // Leave out the runs at either end: a marker may cut them short.
    const int16_t* d = a->sd.data;
    bool lv = d[i0] > 0;
    while(i0 <= i1 && (d[i0] > 0) == lv)
        i0++;

    lv = d[i1] > 0;
    while(i1 > i0 && (d[i1] > 0) == lv)
        i1--;

    if(i0 >= i1) return;

    a->info_te = info_te(d, i0, i1);
    if(a->info_te) a->info_enc = info_enc(d, i0, i1, a->info_te);
}
