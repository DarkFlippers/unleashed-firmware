/* Decode: A..B is replayed through the firmware's own Sub-GHz decoders - the
 * same ones the Sub-GHz app uses while reading - so every protocol it knows
 * works here with no per-protocol code. Decoders are allocated one at a time
 * instead of through a SubGhzReceiver (which holds all of them at once,
 * BinRAW's big buffers included), keeping peak RAM to a single instance.
 * Identical frames from one protocol collapse into one entry; only a short
 * fingerprint is kept per entry and the full frame is recovered on demand by
 * replaying that one protocol. */

#include "app.h"

// FNV-1a of a frame description, skipping measured timings ("Te:350us",
// "GT:Te*31") that jitter between otherwise identical frames.
static uint32_t decoded_hash(const char* s) {
    uint32_t h = 2166136261u;
    while(*s) {
        if(s[0] == 'T' && s[1] == 'e' && (s[2] == ':' || s[2] == '*')) {
            for(s += 3; *s >= '0' && *s <= '9'; s++)
                ;
            continue;
        }

        h = (h ^ (uint8_t)*s++) * 16777619u;
    }

    return h;
}

static void decoded_key(const char* s, char* out, size_t n) {
    size_t len = 0;
    const char* k = strstr(s, "Key:");
    if(k) {
        for(k += 4; *k == ' '; k++)
            ;

        if(k[0] == '0' && (k[1] == 'x' || k[1] == 'X')) k += 2;

        while(len + 1 < n && *k && strchr("0123456789ABCDEFabcdef", *k))
            out[len++] = *k++;
    }

    out[len] = '\0';
}

static void decode_rx_cb(SubGhzProtocolDecoderBase* dec, void* ctx) {
    DecodeCtx* dc = ctx;
    furi_string_reset(dc->text);
    subghz_protocol_decoder_base_get_string(dec, dc->text);
    uint32_t h = decoded_hash(furi_string_get_cstr(dc->text));
#ifndef FW_ORIGIN_Official
    // Custom firmware's KeeLoq sticks to the manufacturer (or "Unknown") of the
    // first frame it decodes; remotes of other makes in A..B need all of them.
    subghz_environment_reset_keeloq(dc->env);
#endif

    if(dc->want) {
        if(h == dc->want->hash) {
            dc->hit = true;
            dc->hit_last = dc->run_gap ? dc->run - 1 : dc->run_end - 1;
        }

        return;
    }

    for(size_t i = 0; i < dc->n; i++) {
        if(dc->list[i].proto == dec->protocol && dc->list[i].hash == h) {
            if(dc->list[i].frames < UINT16_MAX) dc->list[i].frames++;

            return;
        }
    }

    if(dc->n >= DECODE_MAX) return;

    Decoded* d = &dc->list[dc->n++];
    *d = (Decoded){.proto = dec->protocol, .hash = h, .frames = 1};
    decoded_key(furi_string_get_cstr(dc->text), d->key, sizeof(d->key));

    /* Where the frame lies, for Locate. A decoder reports a frame once the
     * run that completes it - usually the gap after it - is fed, so the last
     * pulse is known. Its start is not reported: take the pulse after the
     * latest gap at least half as long as the one closing the frame (or as
     * the signal's guard), which skips gaps inside the frame such as the
     * header gap of KeeLoq. A high run that long ends the search too: it is
     * a carrier or a stuck receiver, not a pulse of the frame. */
    d->last = dc->run_gap ? dc->run - 1 : dc->run_end - 1;
    uint32_t thr = (dc->run_gap >= DECODE_LEAD_MIN_US ? dc->run_gap : dc->lead_us) / 2;
    uint32_t low = 0, high = 0;
    size_t after = d->last;
    d->first = d->last;
    for(size_t i = d->last; i-- > dc->i0;) {
        int16_t v = dc->data[i];
        if(v <= 0) {
            low += -v;
            high = 0;
            continue;
        }

        if(low >= thr) break;

        if(!high) after = d->first;

        low = 0;
        high += v;
        if(high >= thr) {
            d->first = after;
            break;
        }

        d->first = i;
    }
}

// Feed one level run [run, end) - extra is silence added past the selection.
static void decode_feed(
    const SubGhzProtocol* p,
    SubGhzProtocolDecoderBase* dec,
    DecodeCtx* dc,
    bool level,
    uint32_t dur,
    uint32_t extra,
    size_t run,
    size_t end) {
    dc->run = run;
    dc->run_end = end;
    dc->run_gap = level ? 0 : dur;
    p->decoder->feed(dec, level, dur + extra);
}

// Feed A..B to one protocol's decoder as level/duration runs. Stops right after
// a replay hit, so the returned (still allocated) decoder carries that frame.
static SubGhzProtocolDecoderBase*
    decode_run(const App* a, const SubGhzProtocol* p, DecodeCtx* dc) {
    SubGhzProtocolDecoderBase* dec = p->decoder->alloc(dc->env);
    dec->callback = decode_rx_cb;
    dec->context = dc;
    p->decoder->reset(dec);
    p->decoder->feed(dec, false, dc->lead_us);

    bool level = a->sd.data[dc->i0] > 0;
    uint32_t dur = 0;
    size_t run = dc->i0;
    for(size_t i = dc->i0; i <= dc->i1 && !dc->hit; i++) {
        int16_t v = a->sd.data[i];
        if((v > 0) != level) {
            decode_feed(p, dec, dc, level, dur, 0, run, i);
            level = v > 0;
            dur = 0;
            run = i;
        }

        dur += iabs32(v);
    }

    // Close the last frame with a long silence so end-of-packet logic fires.
    if(!dc->hit && level) {
        decode_feed(p, dec, dc, true, dur, 0, run, dc->i1 + 1);
        dur = 0;
        run = dc->i1 + 1;
    }

    if(!dc->hit) decode_feed(p, dec, dc, false, dur, DECODE_TAIL_US, run, dc->i1 + 1);

    return dec;
}

/* Shadow silence fed ahead of A..B (never shown or saved), standing in for
 * whatever comes before the first pulse. Preamble-less protocols (Princeton,
 * CAME, Nice FLO, ...) only start on a guard gap inside a protocol-specific
 * window around N x Te, and those windows do not all overlap, so no fixed
 * value fits every protocol. The signal's own guard does: take the longest
 * gap in the selection that could be one, else a value most windows accept. */
static uint32_t decode_lead_us(const App* a, size_t i0, size_t i1) {
    uint32_t best = 0, run = 0;
    for(size_t i = i0; i <= i1 + 1; i++) {
        if(i <= i1 && a->sd.data[i] <= 0) {
            run += -a->sd.data[i];
            continue;
        }

        if(run >= DECODE_LEAD_MIN_US && run <= DECODE_LEAD_MAX_US && run > best) best = run;

        run = 0;
    }

    return best ? best : DECODE_LEAD_US;
}

// Free RAM in all and in one piece: malloc crashes rather than fail.
static bool heap_ok(size_t total, size_t block) {
    return memmgr_get_free_heap() >= total && memmgr_heap_get_max_free_block() >= block;
}

static void decode_selection(const App* a, DecodeCtx* dc) {
    size_t count = subghz_protocol_registry_count(&subghz_protocol_registry);
    for(size_t k = 0; k < count; k++) {
        // BinRAW only captures once the receiver reports RSSI, which a replay has
        // not got; on official firmware its decoder also takes ~10 KB.
        const SubGhzProtocol* p =
            subghz_protocol_registry_get_by_index(&subghz_protocol_registry, k);
        if(!(p->flag & SubGhzProtocolFlag_Decodable) || (p->flag & SubGhzProtocolFlag_BinRAW) ||
           !p->decoder || !p->decoder->alloc)
            continue;

        if(!heap_ok(DECODE_RUN_HEAP, DECODE_RUN_BLOCK)) {
            dc->oom = true;
            break;
        }

        p->decoder->free(decode_run(a, p, dc));
    }
}

// Replay A..B through d's protocol until d's frame comes out again. On success
// dc->text describes it and the returned decoder (caller frees) still holds it.
static SubGhzProtocolDecoderBase* decoded_replay(const App* a, DecodeCtx* dc, const Decoded* d) {
    dc->want = d;
    dc->hit = false;
    SubGhzProtocolDecoderBase* dec = decode_run(a, d->proto, dc);
    if(dc->hit) return dec;

    d->proto->decoder->free(dec);
    return NULL;
}

/* The start decode_rx_cb guesses from gap lengths goes wrong both ways: a gap
 * inside the frame half as long as the one closing it cuts it short (KeeLoq's
 * 10 Te header gap before a short guard: no preamble, so it does not decode
 * again), and a guard shorter than that half lets in whatever came before.
 * Let d's own decoder say instead: the frame starts at the latest burst (pulses
 * between gaps of at least DECODE_LEAD_MIN_US) from which d's frame comes out
 * again - as this one, not an identical frame before it. The guess stays when
 * none does. */
static void decoded_start(const App* a, DecodeCtx* dc, Decoded* d) {
    size_t i0 = dc->i0, i1 = dc->i1, j = d->last + 1;
    for(int tries = 0; tries < DECODE_START_TRIES && j > i0; tries++) {
        // Back over the burst ending at j - 1, to the gap before it
        size_t start = j;
        for(uint32_t low = 0; j > i0; j--) {
            int16_t v = a->sd.data[j - 1];
            if(v > 0) {
                low = 0;
                start = j - 1;
            } else if((low += -v) >= DECODE_LEAD_MIN_US)
                break;
        }

        dc->i0 = start;
        dc->i1 = d->last;
        SubGhzProtocolDecoderBase* dec = decoded_replay(a, dc, d);
        if(dec) {
            d->proto->decoder->free(dec);
            if(dc->hit_last == d->last) d->first = start;

            break;
        }

        while(j > i0 && a->sd.data[j - 1] <= 0)
            j--;
    }

    dc->i0 = i0;
    dc->i1 = i1;
}

// Move A..B onto d's first frame, found while decoding.
static void decoded_locate(App* a, DecodeCtx* dc, Decoded* d) {
    decoded_start(a, dc, d);

    int32_t t = a->sd.win_t0, t_a = t, t_b = t, gap = 0;
    for(size_t i = 0; i < a->sd.count; i++) {
        if(i == d->first) t_a = t;

        t += iabs32(a->sd.data[i]);
        if(i == d->last)
            t_b = t;
        else if(i > d->last) {
            if(a->sd.data[i] > 0) break;

            gap = t - t_b;
        }
    }

    furi_mutex_acquire(a->mutex, FuriWaitForever);
    select_frame(a, t_a, t_b, gap);
    recompute_activity(a);
    furi_mutex_release(a->mutex);
}

// The firmware serializer takes the short preset names the Sub-GHz app uses.
static const char* preset_short_name(const char* full) {
    static const char* const map[][2] = {
        {"FuriHalSubGhzPresetOok270Async", "AM270"},
        {"FuriHalSubGhzPresetOok650Async", "AM650"},
        {"FuriHalSubGhzPreset2FSKDev238Async", "FM238"},
        {"FuriHalSubGhzPreset2FSKDev12KAsync", "FM12K"},
        {"FuriHalSubGhzPreset2FSKDev476Async", "FM476"},
    };

    for(size_t i = 0; i < COUNT_OF(map); i++)
        if(!strcmp(full, map[i][0])) return map[i][1];

    return full;
}

// Save a decoded entry as a regular Sub-GHz key file (the same format the
// Sub-GHz app writes after Read), ready to be emulated.
bool write_decoded(Storage* st, App* a, DecodeCtx* dc, const Decoded* d, const char* path) {
    SubGhzProtocolDecoderBase* dec = decoded_replay(a, dc, d);
    bool ok = false;
    if(dec) {
        SubGhzRadioPreset preset = {
            .name = furi_string_alloc_set(preset_short_name(a->sd.preset)),
            .frequency = a->sd.frequency,
        };

        FlipperFormat* ff = flipper_format_file_alloc(st);
        bool opened = flipper_format_file_open_always(ff, path);
        ok = opened &&
             subghz_protocol_decoder_base_serialize(dec, ff, &preset) == SubGhzProtocolStatusOk;

        flipper_format_free(ff);
        furi_string_free(preset.name);
        dec->protocol->decoder->free(dec);

        if(opened && !ok) storage_simply_remove(st, path);
    }

    if(!ok) snprintf(a->status, sizeof(a->status), "Write failed");

    return ok;
}

// <Protocol>[_<Manufacturer>] from the decoded text, made filename-safe.
void decoded_base_name(const DecodeCtx* dc, const Decoded* d, char* out, size_t outlen) {
    size_t n = strlcpy(out, d->proto->name, outlen);
    const char* mf = strstr(furi_string_get_cstr(dc->text), "MF:");
    if(mf) {
        mf += 3;
        int len = strcspn(mf, " \n");
        if(len && strncmp(mf, "Unknown", len) != 0 && n < outlen)
            snprintf(out + n, outlen - n, "_%.*s", len, mf);
    }

    char* w = out;
    for(const char* r = out; *r; r++) {
        char l = *r | 0x20;
        char c = ((*r >= '0' && *r <= '9') || (l >= 'a' && l <= 'z') || *r == '-') ? *r : '_';
        if(c != '_' || (w > out && w[-1] != '_')) *w++ = c;
    }

    while(w > out && w[-1] == '_')
        w--;
    *w = '\0';
}

// Full description of one decoded entry. Returns the button pressed (Left:
// Locate, Right: Save) or -1 for Back.
static int decoded_show(Gui* gui, const App* a, DecodeCtx* dc, const Decoded* d) {
    SubGhzProtocolDecoderBase* dec = decoded_replay(a, dc, d);
    if(!dec) return -1;

    dec->protocol->decoder->free(dec);
    furi_string_replace_all(dc->text, "\r", "");

    PromptCtx pc = {.index = UINT32_MAX};
    Widget* w = widget_alloc();
    widget_add_text_scroll_element(w, 0, 0, 128, 50, furi_string_get_cstr(dc->text));
    widget_add_button_element(w, GuiButtonTypeLeft, "Locate", button_ok_cb, &pc);
    widget_add_button_element(w, GuiButtonTypeRight, "Save", button_ok_cb, &pc);
    prompt_run(gui, &pc, widget_get_view(w));
    widget_free(w);
    return pc.index == UINT32_MAX ? -1 : (int)pc.index;
}

// List of decoded entries (skipped for a single one). Returns the entry picked
// for Save, or for Locate with *locate set, or -1.
static int32_t decoded_browse(Gui* gui, const App* a, DecodeCtx* dc, bool* locate) {
    uint32_t sel = 0;
    while(true) {
        if(dc->n > 1) {
            PromptCtx pc = {0};
            Submenu* sm = submenu_alloc();
            char label[48];
            snprintf(label, sizeof(label), "Decoded: %u", (unsigned)dc->n);
            submenu_set_header(sm, label);

            for(size_t i = 0; i < dc->n; i++) {
                const Decoded* d = &dc->list[i];
                int len = snprintf(label, sizeof(label), "%s %s", d->proto->name, d->key);
                if(d->frames > 1 && len > 0 && (size_t)len < sizeof(label))
                    snprintf(label + len, sizeof(label) - len, " x%u", (unsigned)d->frames);

                submenu_add_item(sm, label, i, pick_index_cb, &pc);
            }

            submenu_set_selected_item(sm, sel);
            bool ok = prompt_run(gui, &pc, submenu_get_view(sm));
            submenu_free(sm);

            if(!ok) return -1;

            sel = pc.index;
        }

        int button = decoded_show(gui, a, dc, &dc->list[sel]);
        if(button >= 0) {
            *locate = button == GuiButtonTypeLeft;
            return (int32_t)sel;
        }

        if(dc->n == 1) return -1;
    }
}

void do_decode_action(Gui* gui, ViewPort* vp, FuriMessageQueue* queue, Storage* st, App* a) {
    DecodeCtx dc = {0};
    a->status_until = furi_get_tick() + 1500;

    if(!selection_range(a, &dc.i0, &dc.i1)) {
        snprintf(a->status, sizeof(a->status), "Nothing in A..B");
        return;
    }

    // The shadow silence replaces any leading gap: one that is too long or cut
    // short by A would fall outside the start window.
    while(dc.i0 < dc.i1 && a->sd.data[dc.i0] <= 0)
        dc.i0++;

    dc.lead_us = decode_lead_us(a, dc.i0, dc.i1);
    dc.data = a->sd.data;

    if(!heap_ok(DECODE_HEAP, DECODE_BLOCK)) {
        snprintf(
            a->status,
            sizeof(a->status),
            "Low memory\nfree %u, block %u",
            (unsigned)memmgr_get_free_heap(),
            (unsigned)memmgr_heap_get_max_free_block());
        return;
    }

    snprintf(a->status, sizeof(a->status), "Decoding...");
    a->status_until = furi_get_tick() + 60000;
    view_port_update(vp);

    dc.env = subghz_env_setup();
    // Rolling codes (KeeLoq & co.) only show their manufacturer and counter
    // with the keystore; without spare RAM they still decode, just less. Its
    // size depends on the key files, so it is dropped again when it leaves
    // Decode short.
    if(heap_ok(DECODE_KEYS_HEAP, DECODE_KEYS_BLOCK)) {
        subghz_environment_load_keystore(dc.env, "/ext/subghz/assets/keeloq_mfcodes");
        subghz_environment_load_keystore(dc.env, "/ext/subghz/assets/keeloq_mfcodes_user");
        if(!heap_ok(DECODE_HEAP, DECODE_BLOCK)) {
            subghz_environment_free(dc.env);
            dc.env = subghz_env_setup();
        }
    }

    dc.list = malloc(DECODE_MAX * sizeof(Decoded));
    dc.text = furi_string_alloc();
    decode_selection(a, &dc);

    int32_t save = -1;
    bool locate = false;
    if(dc.n) {
        // "Decoding..." stays up underneath: picking an entry replays it before showing it
        save = decoded_browse(gui, a, &dc, &locate);
        furi_message_queue_reset(queue);
        a->status_until = 0;
    } else {
        snprintf(a->status, sizeof(a->status), dc.oom ? "Low memory" : "No known protocol");
        a->status_until = furi_get_tick() + 1500;
    }

    if(save >= 0 && locate) {
        decoded_locate(a, &dc, &dc.list[save]);
        snprintf(a->status, sizeof(a->status), "Located");
        a->status_until = furi_get_tick() + 1500;
    } else if(save >= 0)
        prompt_and_save(gui, vp, queue, st, a, &dc, &dc.list[save]);

    furi_string_free(dc.text);
    free(dc.list);
    subghz_environment_free(dc.env);
}
