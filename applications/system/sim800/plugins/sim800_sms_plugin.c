#include "sim800_sms_plugin.h"
#include "sim800_cyrillic_font_i.h"

#include <flipper_application/flipper_application.h>
#include <gui/elements.h>
#include <stdio.h>
#include <string.h>

#define SMS_INBOX_VISIBLE 4
#define SMS_INBOX_COLS    23
#define SMS_READER_LINES  4
#define SMS_READER_COLS   24
#define SMS_TEXT_LINES    5

static int sim800_hex_nibble(char c) {
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'A' && c <= 'F') return 10 + (c - 'A');
    if(c >= 'a' && c <= 'f') return 10 + (c - 'a');
    return -1;
}

static bool sim800_ucs2_unit_valid(uint16_t u) {
    if(u == 0x0009 || u == 0x000A || u == 0x000D) return true;
    if(u >= 0x0020 && u <= 0x007E) return true; // ASCII
    if(u >= 0x00A0 && u <= 0x024F) return true; // Latin-1 / Extended
    if(u >= 0x0300 && u <= 0x036F) return true; // Combining diacritics
    if(u >= 0x0400 && u <= 0x052F) return true; // Cyrillic
    if(u >= 0x2000 && u <= 0x2BFF) return true; // Punctuation, currency, symbols, arrows
    if(u >= 0xD800 && u <= 0xDFFF) return true; // UTF-16 surrogates
    if(u >= 0xFE00 && u <= 0xFE0F) return true; // Variation selectors
    if(u >= 0xFF00 && u <= 0xFFEF) return true; // Fullwidth forms
    return false;
}

static bool sim800_is_ucs2_hex_n(const char* str, size_t len) {
    if(len < 4 || (len % 4) != 0) return false;

    bool has_common = false;
    for(size_t i = 0; i < len; i += 4) {
        int n0 = sim800_hex_nibble(str[i]);
        int n1 = sim800_hex_nibble(str[i + 1]);
        int n2 = sim800_hex_nibble(str[i + 2]);
        int n3 = sim800_hex_nibble(str[i + 3]);
        if(n0 < 0 || n1 < 0 || n2 < 0 || n3 < 0) return false;

        uint16_t u = (uint16_t)((n0 << 12) | (n1 << 8) | (n2 << 4) | n3);
        if(!sim800_ucs2_unit_valid(u)) return false;
        if(u <= 0x00FF || (u >= 0x0400 && u <= 0x052F)) has_common = true;
    }

    return has_common;
}

static void sim800_append_utf8(FuriString* out, uint16_t u) {
    switch(u) {
    case 0x000D:
    case 0x00AD: // soft hyphen
    case 0x200B: // zero-width space
    case 0x200C:
    case 0x200D:
    case 0x200E:
    case 0x200F:
    case 0xFEFF:
        return;
    case 0x0009:
    case 0x00A0: // NBSP
    case 0x2000:
    case 0x2001:
    case 0x2002:
    case 0x2003:
    case 0x2004:
    case 0x2005:
    case 0x2006:
    case 0x2007: // figure space
    case 0x2008:
    case 0x2009: // thin space
    case 0x200A:
    case 0x202F: // narrow NBSP (Russian thousand separator)
    case 0x205F:
        furi_string_push_back(out, ' ');
        return;
    case 0x00AB: // «
    case 0x00BB: // »
    case 0x201C: // “
    case 0x201D: // ”
    case 0x201E: // „
    case 0x201F:
        furi_string_push_back(out, '"');
        return;
    case 0x2018: // ‘
    case 0x2019: // ’
    case 0x201A: // ‚
    case 0x2032: // ′
        furi_string_push_back(out, '\'');
        return;
    case 0x2039: // ‹
    case 0x25C0: // ◀
    case 0x25C2: // ◂
    case 0x25C4: // ◄
    case 0x276E: // ❮
        furi_string_push_back(out, '<');
        return;
    case 0x203A: // ›
    case 0x25B6: // ▶
    case 0x25B8: // ▸
    case 0x25BA: // ►
    case 0x276F: // ❯
        furi_string_push_back(out, '>');
        return;
    case 0x2192: // → rightwards arrow
    case 0x21D2: // ⇒ rightwards double arrow
    case 0x2794: // ➔
    case 0x279C: // ➜
    case 0x279D: // ➝
    case 0x279E: // ➞
    case 0x279F: // ➟
    case 0x27A1: // ➡
        furi_string_cat_str(out, "->");
        return;
    case 0x2190: // ← leftwards arrow
    case 0x21D0: // ⇐
        furi_string_cat_str(out, "<-");
        return;
    case 0x2191: // ↑
        furi_string_push_back(out, '^');
        return;
    case 0x2193: // ↓
        furi_string_push_back(out, 'v');
        return;
    case 0x2010: // ‐ hyphen
    case 0x2011: // ‑ non-breaking hyphen
    case 0x2012: // ‒ figure dash
    case 0x2013: // – en dash
    case 0x2014: // — em dash
    case 0x2015: // ― horizontal bar
    case 0x2212: // − minus sign
    case 0xFE63: // ﹣ small hyphen-minus
    case 0xFF0D: // － fullwidth hyphen-minus
        furi_string_push_back(out, '-');
        return;
    case 0x2022: // • bullet (masked card digits)
    case 0x25CF: // ● black circle
        furi_string_push_back(out, '*');
        return;
    case 0x00B7: // · middle dot
        furi_string_push_back(out, '.');
        return;
    case 0x00D7: // × multiplication
        furi_string_push_back(out, 'x');
        return;
    case 0x00B1: // ±
        furi_string_cat_str(out, "+/-");
        return;
    case 0x00A9: // ©
        furi_string_cat_str(out, "(c)");
        return;
    case 0x00AE: // ®
        furi_string_cat_str(out, "(R)");
        return;
    case 0x2122: // ™
        furi_string_cat_str(out, "(TM)");
        return;
    case 0x203C: // ‼
        furi_string_cat_str(out, "!!");
        return;
    case 0x2049: // ⁉
        furi_string_cat_str(out, "!?");
        return;
    case 0x2713: // ✓
    case 0x2714: // ✔
    case 0x2705: // ✅
        furi_string_cat_str(out, "[v]");
        return;
    case 0x2715: // ✕
    case 0x2716: // ✖
    case 0x2717: // ✗
    case 0x274C: // ❌
        furi_string_cat_str(out, "[x]");
        return;
    case 0x2264: // ≤
        furi_string_cat_str(out, "<=");
        return;
    case 0x2265: // ≥
        furi_string_cat_str(out, ">=");
        return;
    case 0x2260: // ≠
        furi_string_cat_str(out, "!=");
        return;
    case 0x2026: // … ellipsis
        furi_string_cat_str(out, "...");
        return;
    case 0x2116: // № numero sign
        furi_string_push_back(out, '#');
        return;
    case 0x20BD: // ₽ ruble
        furi_string_cat_str(out, "RUB");
        return;
    case 0x20AC: // € euro
        furi_string_cat_str(out, "EUR");
        return;
    case 0x00A3: // £ pound
        furi_string_cat_str(out, "GBP");
        return;
    default:
        break;
    }

    if((u >= 0xFE00 && u <= 0xFE0F) || (u >= 0x0300 && u <= 0x036F)) return;

    if(u < 0x0080) {
        furi_string_push_back(out, (char)u);
    } else if(u >= 0x0400 && u <= 0x04FF) {
        furi_string_push_back(out, (char)(0xC0 | (u >> 6)));
        furi_string_push_back(out, (char)(0x80 | (u & 0x3F)));
    } else {
        furi_string_push_back(out, '?');
    }
}

static void sim800_decode_ucs2_chunk(const char* in, size_t len, FuriString* out) {
    if(!sim800_is_ucs2_hex_n(in, len)) {
        for(size_t i = 0; i < len; i++) {
            furi_string_push_back(out, in[i]);
        }
        return;
    }

    for(size_t i = 0; i < len; i += 4) {
        uint16_t u = (uint16_t)((sim800_hex_nibble(in[i]) << 12) |
                                (sim800_hex_nibble(in[i + 1]) << 8) |
                                (sim800_hex_nibble(in[i + 2]) << 4) |
                                sim800_hex_nibble(in[i + 3]));

        // Skip UTF-16 surrogate pairs cleanly (emoji)
        if(u >= 0xD800 && u <= 0xDBFF) {
            if(i + 4 < len) i += 4;
            furi_string_push_back(out, '?');
            continue;
        }
        if(u >= 0xDC00 && u <= 0xDFFF) continue;

        sim800_append_utf8(out, u);
    }
}

static void sim800_sms_decode_text(const char* in, FuriString* out) {
    furi_assert(in);
    furi_assert(out);

    furi_string_reset(out);

    // Decode line by line so multi-line bodies or mixed ASCII/UCS2 chunks both work
    const char* p = in;
    while(*p != '\0') {
        const char* nl = strchr(p, '\n');
        size_t len = nl ? (size_t)(nl - p) : strlen(p);
        while(len > 0 && p[len - 1] == '\r') {
            len--;
        }
        sim800_decode_ucs2_chunk(p, len, out);
        if(!nl) break;
        furi_string_push_back(out, '\n');
        p = nl + 1;
    }
}

/** Advance pointer by one UTF-8 codepoint. */
static const char* sim800_utf8_next(const char* p) {
    unsigned char c = (unsigned char)*p;
    if(c == 0) return p;
    if((c & 0x80) == 0) return p + 1;
    if((c & 0xE0) == 0xC0) return (p[1] != '\0') ? p + 2 : p + 1;
    if((c & 0xF0) == 0xE0) return (p[1] && p[2]) ? p + 3 : p + 1;
    if((c & 0xF8) == 0xF0) return (p[1] && p[2] && p[3]) ? p + 4 : p + 1;
    return p + 1;
}

/** Copy up to @p max_chars UTF-8 codepoints into @p dst, flattening newlines and adding ".." if clipped. */
static void sim800_utf8_preview(
    const char* from,
    const char* body,
    char* dst,
    size_t dst_size,
    size_t max_chars) {
    size_t out_pos = 0;
    size_t chars = 0;
    bool clipped = false;

    const char* parts[3] = {from, ": ", body};
    for(size_t part = 0; part < 3 && !clipped; part++) {
        const char* p = parts[part];
        while(*p != '\0') {
            if(chars >= max_chars) {
                clipped = true;
                break;
            }
            const char* next = sim800_utf8_next(p);
            size_t clen = (size_t)(next - p);
            if(clen == 1 && (*p == '\n' || *p == '\r')) {
                if(out_pos + 1 >= dst_size) {
                    clipped = true;
                    break;
                }
                dst[out_pos++] = ' ';
            } else {
                if(out_pos + clen >= dst_size) {
                    clipped = true;
                    break;
                }
                memcpy(dst + out_pos, p, clen);
                out_pos += clen;
            }
            chars++;
            p = next;
        }
    }

    dst[out_pos] = '\0';

    if(clipped && max_chars >= 3) {
        size_t keep = max_chars - 2;
        const char* p = dst;
        for(size_t i = 0; i < keep && *p != '\0'; i++) {
            p = sim800_utf8_next(p);
        }
        size_t cut = (size_t)(p - dst);
        if(cut + 3 <= dst_size) {
            dst[cut] = '.';
            dst[cut + 1] = '.';
            dst[cut + 2] = '\0';
        }
    }
}

static void sim800_utf8_clip(const char* src, char* dst, size_t dst_size, size_t max_chars) {
    const char* p = src ? src : "";
    size_t out_pos = 0;
    size_t cols = 0;
    while(*p != '\0' && *p != '\n' && *p != '\r' && cols < max_chars) {
        const char* next = sim800_utf8_next(p);
        size_t clen = (size_t)(next - p);
        if(out_pos + clen >= dst_size) break;
        memcpy(dst + out_pos, p, clen);
        out_pos += clen;
        cols++;
        p = next;
    }
    dst[out_pos] = '\0';
}

/** Word-wrap @p text into lines of @p max_cols UTF-8 codepoints.
 *  When @p canvas is NULL, only counts total wrapped lines without drawing.
 */
static uint16_t sim800_utf8_layout(
    const char* text,
    size_t max_cols,
    Canvas* canvas,
    uint8_t x,
    uint8_t y_start,
    uint8_t line_height,
    uint16_t scroll_line,
    uint16_t max_visible,
    uint16_t line_index) {
    char line_buf[96];
    const char* p = text;

    while(*p != '\0') {
        if(*p == '\r') {
            p++;
            continue;
        }
        if(*p == '\n') {
            line_index++;
            p++;
            continue;
        }

        const char* line_start = p;
        const char* last_space = NULL;
        size_t cols = 0;

        while(*p != '\0' && *p != '\n' && *p != '\r' && cols < max_cols) {
            if(*p == ' ') last_space = p;
            p = sim800_utf8_next(p);
            cols++;
        }

        const char* line_end = p;
        if(*p != '\0' && *p != '\n' && *p != '\r' && last_space != NULL &&
           last_space > line_start) {
            line_end = last_space;
            p = last_space + 1;
        } else if(*p == '\r') {
            p++;
            if(*p == '\n') p++;
        } else if(*p == '\n') {
            p++;
        }

        if(canvas && line_index >= scroll_line && line_index < (scroll_line + max_visible)) {
            size_t bytes = (size_t)(line_end - line_start);
            if(bytes >= sizeof(line_buf)) bytes = sizeof(line_buf) - 1;
            memcpy(line_buf, line_start, bytes);
            line_buf[bytes] = '\0';
            uint8_t y = y_start + (uint8_t)((line_index - scroll_line) * line_height);
            canvas_draw_str(canvas, x, y, line_buf);
        }

        line_index++;
    }

    return line_index;
}

static void sim800_sms_draw_inbox(
    Canvas* canvas,
    const Sim800Inbox* inbox,
    size_t selected,
    const char* status_text) {
    if(!canvas) return;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    char header[28];
    if(inbox && inbox->count > 0) {
        snprintf(header, sizeof(header), "SMS inbox (%u)", (unsigned)inbox->count);
    } else {
        snprintf(header, sizeof(header), "SMS inbox");
    }
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 9, header);
    canvas_draw_line(canvas, 0, 11, 127, 11);

    canvas_set_custom_u8g2_font(canvas, u8g2_font_5x8_t_cyrillic);

    if(!inbox || inbox->count == 0) {
        const char* msg = (status_text && status_text[0] != '\0') ? status_text : "Inbox is empty";
        sim800_utf8_layout(msg, SMS_READER_COLS, canvas, 2, 26, 10, 0, 3, 0);
    } else {
        if(selected >= inbox->count) selected = inbox->count - 1;

        size_t offset = 0;
        if(inbox->count > SMS_INBOX_VISIBLE) {
            if(selected > 1) offset = selected - 1;
            if(offset + SMS_INBOX_VISIBLE > inbox->count) {
                offset = inbox->count - SMS_INBOX_VISIBLE;
            }
        }

        char row_text[96];
        uint8_t row_w = (inbox->count > SMS_INBOX_VISIBLE) ? 122 : 127;

        for(size_t slot = 0; slot < SMS_INBOX_VISIBLE; slot++) {
            size_t idx = offset + slot;
            if(idx >= inbox->count) break;

            uint8_t y = (uint8_t)(12 + slot * 10);
            bool is_sel = (idx == selected);

            if(is_sel) {
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_rbox(canvas, 0, y, row_w, 10, 1);
                canvas_set_color(canvas, ColorWhite);
            } else {
                canvas_set_color(canvas, ColorBlack);
            }

            sim800_utf8_preview(
                furi_string_get_cstr(inbox->items[idx].from),
                furi_string_get_cstr(inbox->items[idx].body),
                row_text,
                sizeof(row_text),
                SMS_INBOX_COLS);
            canvas_draw_str(canvas, 2, y + 8, row_text);
        }

        canvas_set_color(canvas, ColorBlack);
        if(inbox->count > SMS_INBOX_VISIBLE) {
            elements_scrollbar_pos(canvas, 127, 12, 40, selected, inbox->count);
        }
    }

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    elements_button_right(canvas, "Refresh");
}

static uint16_t sim800_sms_draw_reader(
    Canvas* canvas,
    const char* from,
    const char* date,
    const char* body,
    uint16_t scroll_line) {
    if(canvas) {
        canvas_clear(canvas);
        canvas_set_color(canvas, ColorBlack);
        canvas_set_custom_u8g2_font(canvas, u8g2_font_5x8_t_cyrillic);

        char header[96];
        sim800_utf8_clip(from, header, sizeof(header), SMS_READER_COLS);
        canvas_draw_str(canvas, 1, 8, header);
        canvas_draw_line(canvas, 0, 10, 127, 10);
    }

    uint16_t total = 0;
    if(date && date[0] != '\0') {
        total = sim800_utf8_layout(
            date, SMS_READER_COLS, canvas, 1, 20, 9, scroll_line, SMS_READER_LINES, total);
    }
    if(body && body[0] != '\0') {
        total = sim800_utf8_layout(
            body, SMS_READER_COLS, canvas, 1, 20, 9, scroll_line, SMS_READER_LINES, total);
    }
    if(total == 0) total = 1;

    if(canvas) {
        if(total > SMS_READER_LINES) {
            uint16_t max_scroll = total - SMS_READER_LINES;
            uint16_t pos = (scroll_line > max_scroll) ? max_scroll : scroll_line;
            elements_scrollbar_pos(canvas, 127, 12, 38, pos, max_scroll + 1);
        }

        canvas_set_font(canvas, FontSecondary);
        elements_button_left(canvas, "Reply");
        elements_button_right(canvas, "Delete");
    }

    return total;
}

static uint16_t sim800_sms_draw_text(
    Canvas* canvas,
    const char* title,
    const char* text,
    uint16_t scroll_line) {
    if(canvas) {
        canvas_clear(canvas);
        canvas_set_color(canvas, ColorBlack);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 2, 9, title ? title : "");
        canvas_draw_line(canvas, 0, 11, 127, 11);
        canvas_set_custom_u8g2_font(canvas, u8g2_font_5x8_t_cyrillic);
    }

    uint16_t total = 0;
    if(text && text[0] != '\0') {
        total = sim800_utf8_layout(
            text, SMS_READER_COLS, canvas, 1, 20, 10, scroll_line, SMS_TEXT_LINES, total);
    }
    if(total == 0) total = 1;

    if(canvas && total > SMS_TEXT_LINES) {
        uint16_t max_scroll = total - SMS_TEXT_LINES;
        uint16_t pos = (scroll_line > max_scroll) ? max_scroll : scroll_line;
        elements_scrollbar_pos(canvas, 127, 13, 50, pos, max_scroll + 1);
    }

    return total;
}

static const Sim800SmsPlugin sim800_sms_plugin = {
    .font = u8g2_font_5x8_t_cyrillic,
    .decode_text = sim800_sms_decode_text,
    .draw_inbox = sim800_sms_draw_inbox,
    .draw_reader = sim800_sms_draw_reader,
    .draw_text = sim800_sms_draw_text,
};

static const FlipperAppPluginDescriptor sim800_sms_plugin_descriptor = {
    .appid = SIM800_SMS_PLUGIN_APP_ID,
    .ep_api_version = SIM800_SMS_PLUGIN_API_VERSION,
    .entry_point = &sim800_sms_plugin,
};

const FlipperAppPluginDescriptor* sim800_sms_plugin_ep(void) {
    return &sim800_sms_plugin_descriptor;
}
